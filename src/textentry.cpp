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
#include "epsrenderer.h"
#include "latexrenderer.h"
#include "jupyterutils.h"

#include "settings.h"

#include <QScopedPointer>
#include <QGraphicsLinearLayout>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <KLocalizedString>

TextEntry::TextEntry(Worksheet* worksheet) : WorksheetEntry(worksheet)
    , m_convertCell(false)
    , m_convertTarget()
    , m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction))
{
    m_textItem->enableRichText(true);
    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &TextEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &TextEntry::moveToNextEntry);
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_textItem, &WorksheetTextItem::doubleClick, this, &TextEntry::resolveImagesAtCursor);
}

void TextEntry::populateMenu(QMenu* menu, QPointF pos)
{
    bool imageSelected = false;
    QTextCursor cursor = m_textItem->textCursor();
    const QChar repl = QChar::ObjectReplacementCharacter;
    if (cursor.hasSelection()) {
        QString selection = m_textItem->textCursor().selectedText();
        imageSelected = selection.contains(repl);
    } else {
        // we need to try both the current cursor and the one after the that
        cursor = m_textItem->cursorForPosition(pos);
        qDebug() << cursor.position();
        for (int i = 2; i; --i) {
            int p = cursor.position();
            if (m_textItem->document()->characterAt(p-1) == repl &&
                cursor.charFormat().hasProperty(EpsRenderer::CantorFormula)) {
                m_textItem->setTextCursor(cursor);
                imageSelected = true;
                break;
            }
            cursor.movePosition(QTextCursor::NextCharacter);
        }
    }
    if (imageSelected) {
        menu->addAction(i18n("Show LaTeX code"), this, SLOT(resolveImagesAtCursor()));
        menu->addSeparator();
    }
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
        m_convertCell = true;
        m_convertTarget = content.attribute(QLatin1String("convertTarget"));
    }
    else
        m_convertCell = false;

    QDomDocument doc = QDomDocument();
    QDomNode n = doc.importNode(content.firstChildElement(QLatin1String("body")), true);
    doc.appendChild(n);
    QString html = doc.toString();
    qDebug() << html;
    m_textItem->setHtml(html);
}

void TextEntry::setContentFromJupyter(const QJsonObject& cell)
{
    if (JupyterUtils::isRawCell(cell))
    {
        m_convertCell = true;

        const QJsonObject& metadata = cell.value(QLatin1String("metadata")).toObject(QJsonObject());
        m_convertTarget = metadata.value(QLatin1String("format")).toString(QString());

        m_textItem->setPlainText(JupyterUtils::getSource(cell));
    }
    else if (JupyterUtils::isMarkdownCell(cell))
    {
        m_convertCell = false;
        m_convertTarget.clear();

        QJsonObject cantorMetadata = JupyterUtils::getCantorMetadata(cell);
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
        if (format.hasProperty(EpsRenderer::CantorFormula))
        {
            showLatexCode(cursor);
        }

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }

    QJsonObject metadata;

    QString entryData;
    QString entryType;

    if (!m_convertCell)
    {
        entryType = QLatin1String("markdown");

        // Jupyter TODO: Handle metadata

        // Add raw text of entry to metadata, for situation when
        // Cantor opens .ipynb converted from our .cws format
        QJsonObject cantorMetadata;
        cantorMetadata.insert(QLatin1String("text_entry_content"), doc->toHtml());
        metadata.insert(JupyterUtils::cantorMetadataKey, cantorMetadata);

        if (Settings::storeTextEntryFormatting())
            entryData = doc->toHtml();
        else
            entryData = doc->toPlainText();
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
    JupyterUtils::setSource(entry, entryData);

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
        if (format.hasProperty(EpsRenderer::CantorFormula))
            showLatexCode(cursor);

        cursor = document->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }

    const QString& html = document->toHtml();
    qDebug() << html;
    QDomElement el = doc.createElement(QLatin1String("Text"));
    QDomDocument myDoc = QDomDocument();
    myDoc.setContent(html);
    el.appendChild(myDoc.documentElement().firstChildElement(QLatin1String("body")));

    if (m_convertCell)
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
    QTextCursor cursor = findLatexCode();
    while (!cursor.isNull())
    {
        QString latexCode = cursor.selectedText();
        qDebug()<<"found latex: "<<latexCode;

        latexCode.remove(0, 2);
        latexCode.remove(latexCode.length() - 2, 2);
        latexCode.replace(QChar::ParagraphSeparator, QLatin1Char('\n')); //Replace the U+2029 paragraph break by a Normal Newline
        latexCode.replace(QChar::LineSeparator, QLatin1Char('\n')); //Replace the line break by a Normal Newline


        Cantor::LatexRenderer* renderer=new Cantor::LatexRenderer(this);
        renderer->setLatexCode(latexCode);
        renderer->setEquationOnly(true);
        renderer->setEquationType(Cantor::LatexRenderer::InlineEquation);
        renderer->setMethod(Cantor::LatexRenderer::LatexMethod);

        renderer->renderBlocking();

        bool success;
        QTextImageFormat formulaFormat;
        if (renderer->renderingSuccessful()) {
            EpsRenderer* epsRend = worksheet()->epsRenderer();
            formulaFormat = epsRend->render(m_textItem->document(), renderer);
            success = !formulaFormat.name().isEmpty();
        } else {
            success = false;
        }

        qDebug()<<"rendering successful? "<<success;
        if (!success) {
            cursor = findLatexCode(cursor);
            continue;
        }

        formulaFormat.setProperty(EpsRenderer::Delimiter, QLatin1String("$$"));

        cursor.insertText(QString(QChar::ObjectReplacementCharacter), formulaFormat);
        delete renderer;

        cursor = findLatexCode(cursor);
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
        QTextCharFormat format = cursor.charFormat();
        if (format.hasProperty(EpsRenderer::CantorFormula))
        {
            qDebug() << "found a formula... rendering the eps...";
            QUrl url = format.property(EpsRenderer::ImagePath).toUrl();
            QSizeF s = worksheet()->epsRenderer()->renderToResource(m_textItem->document(), url);
            qDebug() << "rendering successful? " << s.isValid();

            //cursor.deletePreviousChar();
            //cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        }

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
    QString latexCode = cursor.charFormat().property(EpsRenderer::Code).toString();
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
