/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "textentry.h"
#include "lib/renderer.h"
#include "latexrenderer.h"
#include "lib/jupyterutils.h"
#include "mathrender.h"
#include "worksheetview.h"

#include "settings.h"

#include <QScopedPointer>
#include <QGraphicsLinearLayout>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <KLocalizedString>
#include <KColorScheme>
#include <QRegularExpression>
#include <QStringList>
#include <QInputDialog>
#include <QActionGroup>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QUuid>

#include <KArchiveDirectory>
#include <KArchiveFile>

namespace
{
void copyImageResources(const QTextDocument* source, QTextDocument* target)
{
    for (QTextBlock block = source->begin(); block.isValid(); block = block.next())
    {
        for (auto it = block.begin(); !it.atEnd(); ++it)
        {
            const auto& format = it.fragment().charFormat();
            if (!format.isImageFormat())
                continue;

            const QUrl url(format.toImageFormat().name());
            target->addResource(QTextDocument::ImageResource, url, source->resource(QTextDocument::ImageResource, url));
        }
    }
}
}

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
    connect(m_textItem, &WorksheetTextItem::receivedFocus, worksheet, QOverload<WorksheetTextItem*>::of(&Worksheet::updateFocusedTextItem));
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_textItem, &WorksheetTextItem::doubleClick, this, &TextEntry::resolveImagesAtCursor);

    // Init raw cell target menus
    // This used only for raw cells, but removing and creating this on conversion more complex
    // that just create them always
    m_targetActionGroup= new QActionGroup(this);
	m_targetActionGroup->setExclusive(true);
	connect(m_targetActionGroup, &QActionGroup::triggered, this, &TextEntry::convertTargetChanged);

    m_targetMenu = new QMenu(i18n("Raw Cell Targets"));
	for (int i = 0; i < standartRawCellTargetNames.size(); ++i)
    {
		QAction* action = new QAction(standartRawCellTargetNames[i], m_targetActionGroup);
		action->setData(standartRawCellTargetMimes[i]);
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

WorksheetEntry::Capabilities TextEntry::capabilities() const
{
    return CellMerging | CellSplitting;
}

bool TextEntry::canSplitCell() const
{
    return m_textItem->isEditable();
}

bool TextEntry::canMergeCellWith(const WorksheetEntry* other) const
{
    const auto* textEntry = qobject_cast<const TextEntry*>(other);
    return textEntry && textEntry->m_rawCell == m_rawCell && (!m_rawCell || textEntry->m_convertTarget == m_convertTarget);
}

bool TextEntry::mergeCellContent(WorksheetEntry* other)
{
    auto* textEntry = qobject_cast<TextEntry*>(other);
    if (!textEntry)
        return false;

    copyImageResources(textEntry->m_textItem->document(), m_textItem->document());
    QTextCursor sourceCursor(textEntry->m_textItem->document());
    sourceCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QTextCursor targetCursor(m_textItem->document());
    targetCursor.movePosition(QTextCursor::End);
    targetCursor.insertText(QLatin1String("\n"));
    targetCursor.insertFragment(QTextDocumentFragment(sourceCursor));
    return true;
}

bool TextEntry::splitCellContent(WorksheetEntry* newEntry)
{
    auto* textEntry = qobject_cast<TextEntry*>(newEntry);
    if (!textEntry || !canSplitCell())
        return false;

    const int position = m_textItem->textCursor().position();
    auto* document = m_textItem->document();
    const auto splitPositions = cellSplitPositions(document->toPlainText(), position);
    if (splitPositions.first < 0)
        return false;

    QTextCursor firstCursor(document);
    firstCursor.setPosition(splitPositions.first, QTextCursor::KeepAnchor);
    QTextCursor secondCursor(document);
    secondCursor.setPosition(splitPositions.second);
    secondCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    const QTextDocumentFragment firstFragment(firstCursor);
    const QTextDocumentFragment secondFragment(secondCursor);

    copyImageResources(document, textEntry->m_textItem->document());
    document->clear();
    QTextCursor targetCursor(document);
    targetCursor.insertFragment(firstFragment);
    QTextCursor newCursor(textEntry->m_textItem->document());
    newCursor.insertFragment(secondFragment);

    textEntry->setJupyterMetadata(jupyterMetadata());
    textEntry->m_rawCell = m_rawCell;
    textEntry->m_convertTarget = m_convertTarget;
    if (m_rawCell)
    {
        const int index = standartRawCellTargetMimes.indexOf(m_convertTarget);
        if (index != -1)
            textEntry->m_targetMenu->actions().at(index)->setChecked(true);
        else
            textEntry->addNewTarget(m_convertTarget);
    }
    return true;
}

void TextEntry::setContent(const QDomElement& content)
{
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
    m_textItem->document()->clearUndoRedoStacks();
}

void TextEntry::setContent(const QDomElement& content, const KZip& file)
{
    setContent(content);

    QString renderedMathTempDirectory;
    const QDomNodeList renderedMath = content.elementsByTagName(QLatin1String("RenderedMath"));
    for (int i = renderedMath.count() - 1; i >= 0; --i)
    {
        const QDomElement mathElement = renderedMath.at(i).toElement();
        const QString code = mathElement.attribute(QLatin1String("code"));
        const QString delimiter = mathElement.attribute(QLatin1String("delimiter"), QLatin1String("$$"));

        bool sourcePositionOk = false;
        const int sourcePosition = mathElement.attribute(QLatin1String("sourcePosition")).toInt(&sourcePositionOk);
        if (!sourcePositionOk || sourcePosition < 0 || sourcePosition >= m_textItem->document()->characterCount())
            continue;

        QTextCursor sourceCursor(m_textItem->document());
        sourceCursor.setPosition(sourcePosition);
        QTextCursor formulaCursor = findLatexCode(sourceCursor);
        if (formulaCursor.isNull() || formulaCursor.selectionStart() != sourcePosition)
            continue;

        QString selectedCode = formulaCursor.selectedText();
        selectedCode.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        selectedCode.replace(QChar::LineSeparator, QLatin1Char('\n'));
        if (selectedCode != delimiter + code + delimiter)
            continue;

        QImage image;
        image.loadFromData(QByteArray::fromBase64(mathElement.text().toLatin1()), "PNG");
        if (image.isNull())
            continue;

        QUrl internal;
        internal.setScheme(QLatin1String("internal"));
        internal.setPath(QUuid::createUuid().toString());
        m_textItem->document()->addResource(QTextDocument::ImageResource, internal, image);

        QTextImageFormat format;
        format.setName(internal.url());
        format.setWidth(mathElement.attribute(QLatin1String("width"), QString::number(image.width())).toDouble());
        format.setHeight(mathElement.attribute(QLatin1String("height"), QString::number(image.height())).toDouble());
        format.setProperty(Cantor::Renderer::CantorFormula, mathElement.attribute(QLatin1String("type")).toInt());
        format.setProperty(Cantor::Renderer::Code, code);
        format.setProperty(Cantor::Renderer::Delimiter, delimiter);

        const int verticalAlignment = mathElement.attribute(QLatin1String("verticalAlignment"), QString::number(QTextCharFormat::AlignBaseline)).toInt();
        format.setVerticalAlignment(static_cast<QTextCharFormat::VerticalAlignment>(verticalAlignment));

        const QString archivePath = mathElement.attribute(QLatin1String("path"));
        const KArchiveDirectory* directory = archivePath.isEmpty() ? nullptr : file.directory();
        const KArchiveEntry* archiveEntry = directory ? directory->entry(archivePath) : nullptr;
        if (archiveEntry && archiveEntry->isFile() && archivePath.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive))
        {
            const auto* archiveFile = static_cast<const KArchiveFile*>(archiveEntry);
            if (renderedMathTempDirectory.isEmpty())
            {
                QTemporaryDir tempDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                                            + QDir::separator() + QLatin1String("cantor_textentry-XXXXXX"));
                if (tempDirectory.isValid())
                {
                    tempDirectory.setAutoRemove(false);
                    renderedMathTempDirectory = tempDirectory.path();
                }
            }

            if (!renderedMathTempDirectory.isEmpty() && archiveFile->copyTo(renderedMathTempDirectory))
            {
                const QString imagePath = renderedMathTempDirectory + QDir::separator() + archiveFile->name();
                format.setProperty(Cantor::Renderer::ImagePath, imagePath);
            }
        }

        formulaCursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
    }
    m_textItem->document()->clearUndoRedoStacks();
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
    // If convertTarget set, it's raw cell from Jupyter and we convert it to Jupyter cell

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
            entryData.remove(QRegularExpression(QStringLiteral("<!DOCTYPE[^>]*>\\n")));

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

QDomElement TextEntry::toXml(QDomDocument& doc)
{
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

QDomElement TextEntry::toXml(QDomDocument& doc, KZip& archive)
{
    QDomElement element = toXml(doc);

    int sourcePositionAdjustment = 0;
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while (!cursor.isNull())
    {
        const QTextImageFormat format = cursor.charFormat().toImageFormat();
        if (format.hasProperty(Cantor::Renderer::CantorFormula))
        {
            const QString code = format.property(Cantor::Renderer::Code).toString();
            QString delimiter = format.property(Cantor::Renderer::Delimiter).toString();
            if (delimiter.isEmpty())
                delimiter = QLatin1String("$$");

            const int sourcePosition = cursor.selectionStart() + sourcePositionAdjustment;
            sourcePositionAdjustment += delimiter.size() * 2 + code.size() - 1;

            const QImage image = m_textItem->document()->resource(QTextDocument::ImageResource, QUrl(format.name())).value<QImage>();
            if (!image.isNull())
            {
                QByteArray data;
                QBuffer buffer(&data);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");

                QDomElement mathElement = doc.createElement(QLatin1String("RenderedMath"));
                mathElement.setAttribute(QLatin1String("code"), code);
                mathElement.setAttribute(QLatin1String("delimiter"), delimiter);
                mathElement.setAttribute(QLatin1String("sourcePosition"), sourcePosition);
                mathElement.setAttribute(QLatin1String("type"), format.intProperty(Cantor::Renderer::CantorFormula));
                mathElement.setAttribute(QLatin1String("width"), format.width());
                mathElement.setAttribute(QLatin1String("height"), format.height());
                mathElement.setAttribute(QLatin1String("verticalAlignment"), static_cast<int>(format.verticalAlignment()));

                const QString imagePath = format.property(Cantor::Renderer::ImagePath).toString();
                if (QFile::exists(imagePath))
                {
                    const QString archivePath = QLatin1String("cantor_textentry_")
                        + QUuid::createUuid().toString(QUuid::WithoutBraces)
                        + QLatin1String(".pdf");
                    if (archive.addLocalFile(imagePath, archivePath))
                        mathElement.setAttribute(QLatin1String("path"), archivePath);
                }

                mathElement.appendChild(doc.createTextNode(QString::fromLatin1(data.toBase64())));
                element.appendChild(mathElement);
            }
        }
        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
    return element;
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

bool TextEntry::replace(const QString& replacement)
{
    QTextCursor cursor = m_textItem->textCursor();

    if (cursor.hasSelection()) {
        cursor.insertText(replacement);
        return true;
    }

    return false;
}

QGraphicsObject* TextEntry::mainTextItem() const
{
    return m_textItem;
}

void TextEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    if (size().width() == w && m_textItem->pos().x() == entry_zone_x && !force)
        return;

    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    m_textItem->setGeometry(entry_zone_x, 0, w - margin - entry_zone_x);
    setSize(QSizeF(m_textItem->width() + margin + entry_zone_x, m_textItem->height() + VerticalMargin));
}

bool TextEntry::wantToEvaluate()
{
    return !m_rawCell && worksheet()->embeddedMathEnabled() && !findLatexCode().isNull();
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

    // Resolve all latex inserts
    QTextCursor cursor(m_textItem->document());
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.insertText(m_textItem->resolveImages(cursor));
}

void TextEntry::convertToTextEntry()
{
    m_rawCell = false;
    m_convertTarget.clear();
}

void TextEntry::convertTargetChanged(QAction* action)
{
    if (action == m_ownTarget)
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
        m_convertTarget = action->data().toString();
    }
}

void TextEntry::addNewTarget(const QString& target)
{
    for (auto* action : m_targetMenu->actions()) {
        if (action->data().toString() == target) {
            action->setChecked(true);
            return;
        }
    }

    QAction* action = new QAction(target, m_targetActionGroup);
    action->setData(target);
    action->setCheckable(true);
    action->setChecked(true);
    m_targetMenu->insertAction(m_targetMenu->actions().last(), action);
}

QString TextEntry::text() const
{
    return m_textItem->toPlainText();
}

void TextEntry::updateAfterSettingsChanges()
{
    WorksheetEntry::updateAfterSettingsChanges();
    m_textItem->updateThemeColors();
}
