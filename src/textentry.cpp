/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "textentry.h"
#include "worksheettextitem.h"
#include "lib/renderer.h"
#include "latexrenderer.h"
#include "lib/jupyterutils.h"
#include "mathrender.h"

#include "settings.h"


#include <QScopedPointer>
#include <QGraphicsLinearLayout>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <KLocalizedString>
#include <KColorScheme>
#include <QStringList>
#include <QInputDialog>

QStringList standartRawCellTargetNames = {QLatin1String("None"), QLatin1String("LaTeX"), QLatin1String("reST"), QLatin1String("HTML"), QLatin1String("Markdown")};
QStringList standartRawCellTargetMimes = {QString(), QLatin1String("text/latex"), QLatin1String("text/restructuredtext"), QLatin1String("text/html"), QLatin1String("text/markdown")};

TextEntry::TextEntry(Worksheet* worksheet) : WorksheetEntry(worksheet)
    , m_rawCell(false)
    , m_convertTarget()
    , m_targetActionGroup(nullptr)
    , m_ownTarget{nullptr}
    , m_targetMenu(nullptr)
    , m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction))
{
    m_textItem->enableRichText(true);
    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &TextEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &TextEntry::moveToNextEntry);
    // Modern syntax of signal/stots don't work on this connection (arguments don't match)
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_textItem, &WorksheetTextItem::doubleClick, this, &TextEntry::resolveImagesAtCursor);

    // Init raw cell target menus
    // This used only for raw cells, but removing and creating this on convertation more complex
    // that just create them always
    m_targetActionGroup= new QActionGroup(this);
	m_targetActionGroup->setExclusive(true);
	connect(m_targetActionGroup, &QActionGroup::triggered, this, &TextEntry::convertTargetChanged);

    m_targetMenu = new QMenu(i18n("Raw Cell Targets"));
	for (const QString& key : standartRawCellTargetNames)
    {
		QAction* action = new QAction(key, m_targetActionGroup);
		action->setCheckable(true);
		m_targetMenu->addAction(action);
	}
	m_ownTarget = new QAction(i18n("Add custom target"), m_targetActionGroup);
    m_ownTarget->setCheckable(true);
	m_targetMenu->addAction(m_ownTarget);
}

TextEntry::~TextEntry()
{
    m_targetMenu->deleteLater();
}

void TextEntry::populateMenu(QMenu* menu, QPointF pos)
{
    if (m_rawCell)
    {
        menu->addAction(i18n("Convert to Text Entry"), this, &TextEntry::convertToTextEntry);
        menu->addMenu(m_targetMenu);
    }
    else
    {
        menu->addAction(i18n("Convert to Raw Cell"), this, &TextEntry::convertToRawCell);

        bool imageSelected = false;
        QTextCursor cursor = m_textItem->textCursor();
        const QChar repl = QChar::ObjectReplacementCharacter;
        if (cursor.hasSelection())
        {
            QString selection = m_textItem->textCursor().selectedText();
            imageSelected = selection.contains(repl);
        }
        else
        {
            // we need to try both the current cursor and the one after the that
            cursor = m_textItem->cursorForPosition(pos);
            for (int i = 2; i; --i)
            {
                int p = cursor.position();
                if (m_textItem->document()->characterAt(p-1) == repl &&
                    cursor.charFormat().hasProperty(Cantor::Renderer::CantorFormula))
                {
                    m_textItem->setTextCursor(cursor);
                    imageSelected = true;
                    break;
                }
                cursor.movePosition(QTextCursor::NextCharacter);
            }
        }

        if (imageSelected)
        {
            menu->addAction(i18n("Show LaTeX code"), this, SLOT(resolveImagesAtCursor()));
        }
    }
    menu->addSeparator();
    WorksheetEntry::populateMenu(menu, pos);
}

bool TextEntry::isEmpty()
{
    return m_textItem->document()->isEmpty();
}

int TextEntry::type() const
{
    return Type;
}

bool TextEntry::acceptRichText()
{
    return true;
}

bool TextEntry::focusEntry(int pos, qreal xCoord)
{
    if (aboutToBeRemoved())
        return false;
    m_textItem->setFocusAt(pos, xCoord);
    return true;
}


void TextEntry::setContent(const QString& content)
{
    m_textItem->setPlainText(content);
}

void TextEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);
    if(content.firstChildElement(QLatin1String("body")).isNull())
        return;

    if (content.hasAttribute(QLatin1String("convertTarget")))
    {
        convertToRawCell();
        m_convertTarget = content.attribute(QLatin1String("convertTarget"));

        // Set current action status
        int idx = standartRawCellTargetMimes.indexOf(m_convertTarget);
        if (idx != -1)
            m_targetMenu->actions()[idx]->setChecked(true);
        else
            addNewTarget(m_convertTarget);
    }
    else
        convertToTextEntry();

    QDomDocument doc = QDomDocument();
    QDomNode n = doc.importNode(content.firstChildElement(QLatin1String("body")), true);
    doc.appendChild(n);
    QString html = doc.toString();
    m_textItem->setHtml(html);
}

void TextEntry::setContentFromJupyter(const QJsonObject& cell)
{
    if (Cantor::JupyterUtils::isRawCell(cell))
    {
        convertToRawCell();

        const QJsonObject& metadata = Cantor::JupyterUtils::getMetadata(cell);
        QJsonValue format = metadata.value(QLatin1String("format"));
        // Also checks "raw_mimetype", because raw cell don't corresponds Jupyter Notebook specification
        // See https://github.com/jupyter/notebook/issues/4730
        if (format.isUndefined())
            format = metadata.value(QLatin1String("raw_mimetype"));
        m_convertTarget = format.toString(QString());

        // Set current action status
        int idx = standartRawCellTargetMimes.indexOf(m_convertTarget);
        if (idx != -1)
            m_targetMenu->actions()[idx]->setChecked(true);
        else
            addNewTarget(m_convertTarget);

        m_textItem->setPlainText(Cantor::JupyterUtils::getSource(cell));

        setJupyterMetadata(metadata);
    }
    else if (Cantor::JupyterUtils::isMarkdownCell(cell))
    {
        convertToTextEntry();

        QJsonObject cantorMetadata = Cantor::JupyterUtils::getCantorMetadata(cell);
        m_textItem->setHtml(cantorMetadata.value(QLatin1String("text_entry_content")).toString());
    }
}

QJsonValue TextEntry::toJupyterJson()
{
    // Simple logic:
    // If convertTarget is empty, it's user maded cell and we convert it to a markdown
    // If convertTarget setted, it's raw cell from Jupyter and we convert it to Jupyter cell

    QTextDocument* doc = m_textItem->document()->clone();
    QTextCursor cursor = doc->find(QString(QChar::ObjectReplacementCharacter));
    while(!cursor.isNull())
    {
        QTextCharFormat format = cursor.charFormat();
        if (format.hasProperty(Cantor::Renderer::CantorFormula))
        {
            showLatexCode(cursor);
        }

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }

    QJsonObject metadata(jupyterMetadata());

    QString entryData;
    QString entryType;

    if (!m_rawCell)
    {
        entryType = QLatin1String("markdown");

        // Add raw text of entry to metadata, for situation when
        // Cantor opens .ipynb converted from our .cws format
        QJsonObject cantorMetadata;

        if (Settings::storeTextEntryFormatting())
        {
            entryData = doc->toHtml();

            // Remove DOCTYPE from html
            entryData.remove(QRegExp(QLatin1String("<!DOCTYPE[^>]*>\\n")));

            cantorMetadata.insert(QLatin1String("text_entry_content"), entryData);
        }
        else
            entryData = doc->toPlainText();

        metadata.insert(Cantor::JupyterUtils::cantorMetadataKey, cantorMetadata);

        // Replace our $$ formulas to $
        entryData.replace(QLatin1String("$$"), QLatin1String("$"));

    }
    else
    {
        entryType = QLatin1String("raw");
        metadata.insert(QLatin1String("format"), m_convertTarget);
        entryData = doc->toPlainText();
    }

    QJsonObject entry;
    entry.insert(QLatin1String("cell_type"), entryType);
    entry.insert(QLatin1String("metadata"), metadata);
    Cantor::JupyterUtils::setSource(entry, entryData);

    return entry;
}

QDomElement TextEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QScopedPointer<QTextDocument> document(m_textItem->document()->clone());

    //make sure that the latex code is shown instead of the rendered formulas
    QTextCursor cursor = document->find(QString(QChar::ObjectReplacementCharacter));
    while(!cursor.isNull())
    {
        QTextCharFormat format = cursor.charFormat();
        if (format.hasProperty(Cantor::Renderer::CantorFormula))
            showLatexCode(cursor);

        cursor = document->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }

    const QString& html = document->toHtml();
    QDomElement el = doc.createElement(QLatin1String("Text"));
    QDomDocument myDoc = QDomDocument();
    myDoc.setContent(html);
    el.appendChild(myDoc.documentElement().firstChildElement(QLatin1String("body")));

    if (m_rawCell)
        el.setAttribute(QLatin1String("convertTarget"), m_convertTarget);

    return el;
}

QString TextEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();
    /*
    // would this be plain enough?
    QTextCursor cursor = m_textItem->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QString text = m_textItem->resolveImages(cursor);
    text.replace(QChar::ParagraphSeparator, '\n');
    text.replace(QChar::LineSeparator, '\n');
    */
    QString text = m_textItem->toPlainText();
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + QLatin1String("\n");
    return commentStartingSeq + text.replace(QLatin1String("\n"), QLatin1String("\n") + commentStartingSeq) + QLatin1String("\n");

}

void TextEntry::interruptEvaluation()
{
}

bool TextEntry::evaluate(EvaluationOption evalOp)
{
    int i = 0;
    if (worksheet()->embeddedMathEnabled() && !m_rawCell)
    {
        // Render math in $$...$$ via Latex
        QTextCursor cursor = findLatexCode();
        while (!cursor.isNull())
        {
            QString latexCode = cursor.selectedText();
            qDebug()<<"found latex: " << latexCode;

            latexCode.remove(0, 2);
            latexCode.remove(latexCode.length() - 2, 2);
            latexCode.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
            latexCode.replace(QChar::LineSeparator, QLatin1Char('\n'));

            MathRenderer* renderer = worksheet()->mathRenderer();
            renderer->renderExpression(++i, latexCode, Cantor::LatexRenderer::InlineEquation, this, SLOT(handleMathRender(QSharedPointer<MathRenderResult>)));

            cursor = findLatexCode(cursor);
        }
    }

    evaluateNext(evalOp);

    return true;
}

void TextEntry::updateEntry()
{
    qDebug() << "update Entry";
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while(!cursor.isNull())
    {
        QTextImageFormat format=cursor.charFormat().toImageFormat();

        if (format.hasProperty(Cantor::Renderer::CantorFormula))
            worksheet()->mathRenderer()->rerender(m_textItem->document(), format);

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
}

void TextEntry::resolveImagesAtCursor()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (!cursor.hasSelection())
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    cursor.insertText(m_textItem->resolveImages(cursor));
}

QTextCursor TextEntry::findLatexCode(const QTextCursor& cursor) const
{
    QTextDocument *doc = m_textItem->document();
    QTextCursor startCursor;
    if (cursor.isNull())
        startCursor = doc->find(QLatin1String("$$"));
    else
        startCursor = doc->find(QLatin1String("$$"), cursor);
    if (startCursor.isNull())
        return startCursor;
    const QTextCursor endCursor = doc->find(QLatin1String("$$"), startCursor);
    if (endCursor.isNull())
        return endCursor;
    startCursor.setPosition(startCursor.selectionStart());
    startCursor.setPosition(endCursor.position(), QTextCursor::KeepAnchor);
    return startCursor;
}

QString TextEntry::showLatexCode(QTextCursor& cursor)
{
    QString latexCode = cursor.charFormat().property(Cantor::Renderer::Code).toString();
    cursor.deletePreviousChar();
    latexCode = QLatin1String("$$") + latexCode + QLatin1String("$$");
    cursor.insertText(latexCode);
    return latexCode;
}

int TextEntry::searchText(const QString& text, const QString& pattern,
                          QTextDocument::FindFlags qt_flags)
{
    Qt::CaseSensitivity caseSensitivity;
    if (qt_flags & QTextDocument::FindCaseSensitively)
        caseSensitivity = Qt::CaseSensitive;
    else
        caseSensitivity = Qt::CaseInsensitive;

    int position;
    if (qt_flags & QTextDocument::FindBackward)
        position = text.lastIndexOf(pattern, -1, caseSensitivity);
    else
        position = text.indexOf(pattern, 0, caseSensitivity);

    return position;
}

WorksheetCursor TextEntry::search(const QString& pattern, unsigned flags,
                                  QTextDocument::FindFlags qt_flags,
                                  const WorksheetCursor& pos)
{
    if (!(flags & WorksheetEntry::SearchText) ||
        (pos.isValid() && pos.entry() != this))
        return WorksheetCursor();

    QTextCursor textCursor = m_textItem->search(pattern, qt_flags, pos);
    int position = 0;
    QTextCursor latexCursor;
    QString latex;
    if (flags & WorksheetEntry::SearchLaTeX) {
        const QString repl = QString(QChar::ObjectReplacementCharacter);
        latexCursor = m_textItem->search(repl, qt_flags, pos);
        while (!latexCursor.isNull()) {
            latex = m_textItem->resolveImages(latexCursor);
            position = searchText(latex, pattern, qt_flags);
            if (position >= 0) {
                break;
            }
            WorksheetCursor c(this, m_textItem, latexCursor);
            latexCursor = m_textItem->search(repl, qt_flags, c);
        }
    }

    if (latexCursor.isNull()) {
        if (textCursor.isNull())
            return WorksheetCursor();
        else
            return WorksheetCursor(this, m_textItem, textCursor);
    } else {
        if (textCursor.isNull() || latexCursor < textCursor) {
            int start = latexCursor.selectionStart();
            latexCursor.insertText(latex);
            QTextCursor c = m_textItem->textCursor();
            c.setPosition(start + position);
            c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                           pattern.length());
            return WorksheetCursor(this, m_textItem, c);
        } else {
            return WorksheetCursor(this, m_textItem, textCursor);
        }
    }
}


void TextEntry::layOutForWidth(qreal w, bool force)
{
    if (size().width() == w && !force)
        return;

    m_textItem->setGeometry(0, 0, w);
    setSize(QSizeF(m_textItem->width(), m_textItem->height() + VerticalMargin));
}

bool TextEntry::wantToEvaluate()
{
    return !findLatexCode().isNull();
}

bool TextEntry::isConvertableToTextEntry(const QJsonObject& cell)
{
    if (!Cantor::JupyterUtils::isMarkdownCell(cell))
        return false;

    QJsonObject cantorMetadata = Cantor::JupyterUtils::getCantorMetadata(cell);
    const QJsonValue& textContentValue = cantorMetadata.value(QLatin1String("text_entry_content"));

    if (!textContentValue.isString())
        return false;

    const QString& textContent = textContentValue.toString();
    const QString& source = Cantor::JupyterUtils::getSource(cell);

    return textContent == source;
}


void TextEntry::handleMathRender(QSharedPointer<MathRenderResult> result)
{
    if (!result->successful)
    {
        qDebug() << "TextEntry: math render failed with message" << result->errorMessage;
        return;
    }

    const QString& code = result->renderedMath.property(Cantor::Renderer::Code).toString();
    const QString& delimiter = QLatin1String("$$");
    QTextCursor cursor = m_textItem->document()->find(delimiter + code + delimiter);
    if (!cursor.isNull())
    {
        m_textItem->document()->addResource(QTextDocument::ImageResource, result->uniqueUrl, QVariant(result->image));
        result->renderedMath.setProperty(Cantor::Renderer::Delimiter, QLatin1String("$$"));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), result->renderedMath);
    }
}

void TextEntry::convertToRawCell()
{
    m_rawCell = true;
    m_targetMenu->actions().at(0)->setChecked(true);

    KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
    m_textItem->setBackgroundColor(scheme.background(KColorScheme::AlternateBackground).color());

    // Resolve all latex inserts
    QTextCursor cursor(m_textItem->document());
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.insertText(m_textItem->resolveImages(cursor));
}

void TextEntry::convertToTextEntry()
{
    m_rawCell = false;
    m_convertTarget.clear();

    KColorScheme scheme = KColorScheme(QPalette::Normal, KColorScheme::View);
    m_textItem->setBackgroundColor(scheme.background(KColorScheme::NormalBackground).color());
}

void TextEntry::convertTargetChanged(QAction* action)
{
    int index = standartRawCellTargetNames.indexOf(action->text());
    if (index != -1)
    {
        m_convertTarget = standartRawCellTargetMimes[index];
    }
    else if (action == m_ownTarget)
    {
        bool ok;
        const QString& target = QInputDialog::getText(worksheet()->worksheetView(), i18n("Cantor"), i18n("Target MIME type:"), QLineEdit::Normal, QString(), &ok);
        if (ok && !target.isEmpty())
        {
            addNewTarget(target);
            m_convertTarget = target;
        }
    }
    else
    {
        m_convertTarget = action->text();
    }
}

void TextEntry::addNewTarget(const QString& target)
{
    QAction* action = new QAction(target, m_targetActionGroup);
    action->setCheckable(true);
    action->setChecked(true);
    m_targetMenu->insertAction(m_targetMenu->actions().last(), action);
}
