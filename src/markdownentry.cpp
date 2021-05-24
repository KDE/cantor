/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Yifei Wu <kqwyfg@gmail.com>
    SPDX-FileCopyrightText: 2019-2021 Alexander Semke <alexander.semke@web.de>
*/

#include "markdownentry.h"
#include "jupyterutils.h"
#include "mathrender.h"
#include <config-cantor.h>
#include "settings.h"
#include "worksheetview.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QImage>
#include <QImageReader>
#include <QBuffer>
#include <QDebug>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QClipboard>
#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>

#include <KLocalizedString>
#include <KZip>
#include <KMessageBox>

#ifdef Discount_FOUND
extern "C" {
#include <mkdio.h>
}
#endif


MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : WorksheetEntry(worksheet),
m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction)),
rendered(false)
{
    m_textItem->enableRichText(false);
    m_textItem->setOpenExternalLinks(true);
    m_textItem->installEventFilter(this);
    m_textItem->setAcceptDrops(true);
    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &MarkdownEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &MarkdownEntry::moveToNextEntry);
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
}

void MarkdownEntry::populateMenu(QMenu* menu, QPointF pos)
{

    QAction* firstAction;
    if (!rendered)
    {
        WorksheetEntry::populateMenu(menu, pos);

        firstAction = menu->actions().at(1); //insert the first action for Markdown after the "Evaluate" action
        QAction* action = new QAction(QIcon::fromTheme(QLatin1String("viewimage")), i18n("Insert Image"));
        connect(action, &QAction::triggered, this, &MarkdownEntry::insertImage);
        menu->insertAction(firstAction, action);
    }
    else
    {
        WorksheetEntry::populateMenu(menu, pos);

        firstAction = menu->actions().at(0);
        QAction* action = new QAction(QIcon::fromTheme(QLatin1String("edit-entry")), i18n("Enter Edit Mode"));
        connect(action, &QAction::triggered, this, &MarkdownEntry::enterEditMode);
        menu->insertAction(firstAction, action);
        menu->insertSeparator(firstAction);
    }

    if (attachedImages.size() != 0)
    {
        QAction* action = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear Attachments"));
        connect(action, &QAction::triggered, this, &MarkdownEntry::clearAttachments);
        menu->insertAction(firstAction, action);
    }
}

bool MarkdownEntry::isEmpty()
{
    return m_textItem->document()->isEmpty();
}

int MarkdownEntry::type() const
{
    return Type;
}

bool MarkdownEntry::acceptRichText()
{
    return false;
}

bool MarkdownEntry::focusEntry(int pos, qreal xCoord)
{
    if (aboutToBeRemoved())
        return false;
    m_textItem->setFocusAt(pos, xCoord);
    return true;
}

void MarkdownEntry::setContent(const QString& content)
{
    rendered = false;
    plain = content;
    setPlainText(plain);
}

void MarkdownEntry::setContent(const QDomElement& content, const KZip& file)
{
    rendered = content.attribute(QLatin1String("rendered"), QLatin1String("1")) == QLatin1String("1");
    QDomElement htmlEl = content.firstChildElement(QLatin1String("HTML"));
    if(!htmlEl.isNull())
        html = htmlEl.text();
    else
    {
        html = QLatin1String("");
        rendered = false; // No html provided. Assume that it hasn't been rendered.
    }
    QDomElement plainEl = content.firstChildElement(QLatin1String("Plain"));
    if(!plainEl.isNull())
        plain = plainEl.text();
    else
    {
        plain = QLatin1String("");
        html = QLatin1String(""); // No plain text provided. The entry shouldn't render anything, or the user can't re-edit it.
    }

    const QDomNodeList& attachments = content.elementsByTagName(QLatin1String("Attachment"));
    for (int x = 0; x < attachments.count(); x++)
    {
        const QDomElement& attachment = attachments.at(x).toElement();
        QUrl url(attachment.attribute(QLatin1String("url")));

        const QString& base64 = attachment.text();
        QImage image;
        image.loadFromData(QByteArray::fromBase64(base64.toLatin1()), "PNG");

        attachedImages.push_back(std::make_pair(url, QLatin1String("image/png")));

        m_textItem->document()->addResource(QTextDocument::ImageResource, url, QVariant(image));
    }

    if(rendered)
        setRenderedHtml(html);
    else
        setPlainText(plain);

    // Handle math after setting html
    const QDomNodeList& maths = content.elementsByTagName(QLatin1String("EmbeddedMath"));
    foundMath.clear();
    for (int i = 0; i < maths.count(); i++)
    {
        const QDomElement& math = maths.at(i).toElement();
        const QString mathCode = math.text();

        foundMath.push_back(std::make_pair(mathCode, false));
    }

    if (rendered)
    {
        markUpMath();

        for (int i = 0; i < maths.count(); i++)
        {
            const QDomElement& math = maths.at(i).toElement();
            bool mathRendered = math.attribute(QLatin1String("rendered")).toInt();
            const QString mathCode = math.text();

            if (mathRendered)
            {
                const KArchiveEntry* imageEntry=file.directory()->entry(math.attribute(QLatin1String("path")));
                if (imageEntry && imageEntry->isFile())
                {
                    const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
                    const QString& dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);
                    imageFile->copyTo(dir);
                    const QString& pdfPath = dir + QDir::separator() + imageFile->name();

                    QString latex;
                    Cantor::LatexRenderer::EquationType type;
                    std::tie(latex, type) = parseMathCode(mathCode);

                    // Get uuid by removing 'cantor_' and '.pdf' extension
                    // len('cantor_') == 7, len('.pdf') == 4
                    QString uuid = pdfPath;
                    uuid.remove(0, 7);
                    uuid.chop(4);

                    bool success;
                    const auto& data = worksheet()->mathRenderer()->renderExpressionFromPdf(pdfPath, uuid, latex, type, &success);
                    if (success)
                    {
                        QUrl internal;
                        internal.setScheme(QLatin1String("internal"));
                        internal.setPath(uuid);
                        setRenderedMath(i+1, data.first, internal, data.second);
                    }
                }
                else if (worksheet()->embeddedMathEnabled())
                    renderMathExpression(i+1, mathCode);
            }
        }
    }

    // Because, all previous actions was on load stage,
    // them shoudl unconverted by user
    m_textItem->document()->clearUndoRedoStacks();
}

void MarkdownEntry::setContentFromJupyter(const QJsonObject& cell)
{
    if (!Cantor::JupyterUtils::isMarkdownCell(cell))
        return;

    // https://nbformat.readthedocs.io/en/latest/format_description.html#cell-metadata
    // There isn't Jupyter metadata for markdown cells, which could be handled by Cantor
    // So we just store it
    setJupyterMetadata(Cantor::JupyterUtils::getMetadata(cell));

    const QJsonObject attachments = cell.value(QLatin1String("attachments")).toObject();
    for (const QString& key : attachments.keys())
    {
        const QJsonValue& attachment = attachments.value(key);
        const QString& mimeKey = Cantor::JupyterUtils::firstImageKey(attachment);
        if (!mimeKey.isEmpty())
        {
            const QImage& image = Cantor::JupyterUtils::loadImage(attachment, mimeKey);

            QUrl resourceUrl;
            resourceUrl.setUrl(QLatin1String("attachment:")+key);
            attachedImages.push_back(std::make_pair(resourceUrl, mimeKey));
            m_textItem->document()->addResource(QTextDocument::ImageResource, resourceUrl, QVariant(image));
        }
    }

    setPlainText(Cantor::JupyterUtils::getSource(cell));
    m_textItem->document()->clearUndoRedoStacks();
}

QDomElement MarkdownEntry::toXml(QDomDocument& doc, KZip* archive)
{
    if(!rendered)
        plain = m_textItem->toPlainText();

    QDomElement el = doc.createElement(QLatin1String("Markdown"));
    el.setAttribute(QLatin1String("rendered"), (int)rendered);

    QDomElement plainEl = doc.createElement(QLatin1String("Plain"));
    plainEl.appendChild(doc.createTextNode(plain));
    el.appendChild(plainEl);

    QDomElement htmlEl = doc.createElement(QLatin1String("HTML"));
    htmlEl.appendChild(doc.createTextNode(html));
    el.appendChild(htmlEl);

    QUrl url;
    QString key;
    for (const auto& data : attachedImages)
    {
        std::tie(url, key) = std::move(data);

        QDomElement attachmentEl = doc.createElement(QLatin1String("Attachment"));
        attachmentEl.setAttribute(QStringLiteral("url"), url.toString());

        const QImage& image = m_textItem->document()->resource(QTextDocument::ImageResource, url).value<QImage>();

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");

        attachmentEl.appendChild(doc.createTextNode(QString::fromLatin1(ba.toBase64())));

        el.appendChild(attachmentEl);
    }

    // If math rendered, then append result .pdf to archive
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    for (const auto& data : foundMath)
    {
        QDomElement mathEl = doc.createElement(QLatin1String("EmbeddedMath"));
        mathEl.setAttribute(QStringLiteral("rendered"), data.second);
        mathEl.appendChild(doc.createTextNode(data.first));

        if (data.second)
        {
            bool foundNeededImage = false;
            while(!cursor.isNull() && !foundNeededImage)
            {
                QTextImageFormat format=cursor.charFormat().toImageFormat();
                if (format.hasProperty(Cantor::Renderer::CantorFormula))
                {
                    const QString& latex = format.property(Cantor::Renderer::Code).toString();
                    const QString& delimiter = format.property(Cantor::Renderer::Delimiter).toString();
                    const QString& code = delimiter + latex + delimiter;
                    if (code == data.first)
                    {
                        const QUrl& url = QUrl::fromLocalFile(format.property(Cantor::Renderer::ImagePath).toString());
                        archive->addLocalFile(url.toLocalFile(), url.fileName());
                        mathEl.setAttribute(QStringLiteral("path"), url.fileName());
                        foundNeededImage = true;
                    }
                }
                cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
            }
        }

        el.appendChild(mathEl);
    }

    return el;
}

QJsonValue MarkdownEntry::toJupyterJson()
{
    QJsonObject entry;

    entry.insert(QLatin1String("cell_type"), QLatin1String("markdown"));

    entry.insert(QLatin1String("metadata"), jupyterMetadata());

    QJsonObject attachments;
    QUrl url;
    QString key;
    for (const auto& data : attachedImages)
    {
        std::tie(url, key) = std::move(data);

        const QImage& image = m_textItem->document()->resource(QTextDocument::ImageResource, url).value<QImage>();
        QString attachmentKey = url.toString().remove(QLatin1String("attachment:"));
        attachments.insert(attachmentKey, Cantor::JupyterUtils::packMimeBundle(image, key));
    }
    if (!attachments.isEmpty())
        entry.insert(QLatin1String("attachments"), attachments);

    Cantor::JupyterUtils::setSource(entry, plain);

    return entry;
}

QString MarkdownEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();

    QString text(plain);

    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + QLatin1String("\n");
    return commentStartingSeq + text.replace(QLatin1String("\n"), QLatin1String("\n") + commentStartingSeq) + QLatin1String("\n");
}

void MarkdownEntry::interruptEvaluation()
{
}

bool MarkdownEntry::evaluate(EvaluationOption evalOp)
{
    if(!rendered)
    {
        if (m_textItem->toPlainText() == plain && !html.isEmpty())
        {
            setRenderedHtml(html);
            rendered = true;
            for (auto iter = foundMath.begin(); iter != foundMath.end(); iter++)
                iter->second = false;
            markUpMath();
        }
        else
        {
            plain = m_textItem->toPlainText();
            rendered = renderMarkdown(plain);
        }
        m_textItem->document()->clearUndoRedoStacks();
    }

    if (rendered && worksheet()->embeddedMathEnabled())
        renderMath();

    evaluateNext(evalOp);
    return true;
}

bool MarkdownEntry::renderMarkdown(QString& plain)
{
#ifdef Discount_FOUND
    QByteArray mdCharArray = plain.toUtf8();
    MMIOT* mdHandle = mkd_string(mdCharArray.data(), mdCharArray.size()+1, 0);
    if(!mkd_compile(mdHandle, MKD_LATEX | MKD_FENCEDCODE | MKD_GITHUBTAGS))
    {
        qDebug()<<"Failed to compile the markdown document";
        mkd_cleanup(mdHandle);
        return false;
    }
    char *htmlDocument;
    int htmlSize = mkd_document(mdHandle, &htmlDocument);
    html = QString::fromUtf8(htmlDocument, htmlSize);

    char *latexData;
    int latexDataSize = mkd_latextext(mdHandle, &latexData);
    QStringList latexUnits = QString::fromUtf8(latexData, latexDataSize).split(QLatin1Char(31), QString::SkipEmptyParts);
    foundMath.clear();

    mkd_cleanup(mdHandle);

    setRenderedHtml(html);

    QTextCursor cursor(m_textItem->document());
    for (const QString& latex : latexUnits)
        foundMath.push_back(std::make_pair(latex, false));

    markUpMath();

    return true;
#else
    Q_UNUSED(plain);

    return false;
#endif
}

void MarkdownEntry::updateEntry()
{
    QTextCursor cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter));
    while(!cursor.isNull())
    {
        QTextImageFormat format=cursor.charFormat().toImageFormat();
        if (format.hasProperty(Cantor::Renderer::CantorFormula))
            worksheet()->mathRenderer()->rerender(m_textItem->document(), format);

        cursor = m_textItem->document()->find(QString(QChar::ObjectReplacementCharacter), cursor);
    }
}

WorksheetCursor MarkdownEntry::search(const QString& pattern, unsigned flags,
                                  QTextDocument::FindFlags qt_flags,
                                  const WorksheetCursor& pos)
{
    if (!(flags & WorksheetEntry::SearchText) ||
        (pos.isValid() && pos.entry() != this))
        return WorksheetCursor();

    QTextCursor textCursor = m_textItem->search(pattern, qt_flags, pos);
    if (textCursor.isNull())
        return WorksheetCursor();
    else
        return WorksheetCursor(this, m_textItem, textCursor);
}

void MarkdownEntry::layOutForWidth(qreal entry_zone_x, qreal w, bool force)
{
    if (size().width() == w && m_textItem->pos().x() == entry_zone_x && !force)
        return;

    const qreal margin = worksheet()->isPrinting() ? 0 : RightMargin;

    m_textItem->setGeometry(entry_zone_x, 0, w - margin - entry_zone_x);
    setSize(QSizeF(m_textItem->width() + margin + entry_zone_x, m_textItem->height() + VerticalMargin));
}

bool MarkdownEntry::eventFilter(QObject* object, QEvent* event)
{
    if(object == m_textItem)
    {
        if(event->type() == QEvent::GraphicsSceneMouseDoubleClick)
        {
            QGraphicsSceneMouseEvent* mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            if(!mouseEvent) return false;
            if(mouseEvent->button() == Qt::LeftButton)
            {
                if (rendered)
                {
                    setPlainText(plain);
                    m_textItem->setCursorPosition(mouseEvent->pos());
                    m_textItem->textCursor().clearSelection();
                    rendered = false;
                    return true;
                }
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            auto* key_event = static_cast<QKeyEvent*>(event);
            if (key_event->matches(QKeySequence::Cancel))
            {
                setRenderedHtml(html);
                for (auto iter = foundMath.begin(); iter != foundMath.end(); iter++)
                    iter->second = false;
                rendered = true;
                markUpMath();
                if (worksheet()->embeddedMathEnabled())
                    renderMath();

                return true;
            }
            if (key_event->matches(QKeySequence::Paste))
            {
                QClipboard *clipboard = QGuiApplication::clipboard();
                const QImage& clipboardImage = clipboard->image();
                if (!clipboardImage.isNull())
                {
                    int idx = 0;
                    static const QString clipboardImageNamePrefix = QLatin1String("clipboard_image_");
                    for (auto& data : attachedImages)
                    {
                        const QString& name = data.first.path();
                        if (name.startsWith(clipboardImageNamePrefix))
                        {
                            bool isIntParsed = false;
                            int parsedIndex = name.right(name.size() - clipboardImageNamePrefix.size()).toInt(&isIntParsed);
                            if (isIntParsed)
                                idx = std::max(idx, parsedIndex);
                        }
                    }
                    idx++;
                    const QString& name = clipboardImageNamePrefix+QString::number(idx);

                    addImageAttachment(name, clipboardImage);
                    return true;
                }
            }
        }
        else if (event->type() == QEvent::GraphicsSceneDrop)
        {
            auto* dragEvent = static_cast<QGraphicsSceneDragDropEvent*>(event);
            const QMimeData* mimeData = dragEvent->mimeData();
            if (mimeData->hasUrls())
            {
                QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();

                for (const QUrl url : mimeData->urls())
                {
                    const QString filename = url.toLocalFile();
                    QFileInfo info(filename);
                    if (supportedFormats.contains(info.completeSuffix().toUtf8()))
                    {
                        QImage image(filename);
                        addImageAttachment(info.fileName(), image);
                        m_textItem->textCursor().insertText(QLatin1String("\n"));
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool MarkdownEntry::wantToEvaluate()
{
    return !rendered;
}

void MarkdownEntry::setRenderedHtml(const QString& html)
{
    m_textItem->setHtml(html);
    m_textItem->denyEditing();
}

void MarkdownEntry::setPlainText(const QString& plain)
{
    QTextDocument* doc = m_textItem->document();
    doc->setPlainText(plain);
    m_textItem->setDocument(doc);
    m_textItem->allowEditing();
}

void MarkdownEntry::renderMath()
{
    QTextCursor cursor(m_textItem->document());
    for (int i = 0; i < (int)foundMath.size(); i++)
        if (foundMath[i].second == false)
            renderMathExpression(i+1, foundMath[i].first);
}

void MarkdownEntry::handleMathRender(QSharedPointer<MathRenderResult> result)
{
    if (!result->successful)
    {
        if (Settings::self()->showMathRenderError())
            KMessageBox::error(worksheetView(), result->errorMessage, i18n("Cantor Math Error"));
        else
            qDebug() << "MarkdownEntry: math render failed with message" << result->errorMessage;
        return;
    }

    setRenderedMath(result->jobId, result->renderedMath, result->uniqueUrl, result->image);
}

void MarkdownEntry::renderMathExpression(int jobId, QString mathCode)
{
    QString latex;
    Cantor::LatexRenderer::EquationType type;
    std::tie(latex, type) = parseMathCode(mathCode);
    if (!latex.isNull())
        worksheet()->mathRenderer()->renderExpression(jobId, latex, type, this, SLOT(handleMathRender(QSharedPointer<MathRenderResult>)));
}

std::pair<QString, Cantor::LatexRenderer::EquationType> MarkdownEntry::parseMathCode(QString mathCode)
{
    static const QLatin1String inlineDelimiter("$");
    static const QLatin1String displayedDelimiter("$$");

    if (mathCode.startsWith(displayedDelimiter) && mathCode.endsWith(displayedDelimiter))
    {
        mathCode.remove(0, 2);
        mathCode.chop(2);

        if (mathCode[0] == QChar(6))
            mathCode.remove(0, 1);

        return std::make_pair(mathCode, Cantor::LatexRenderer::FullEquation);
    }
    else if (mathCode.startsWith(inlineDelimiter) && mathCode.endsWith(inlineDelimiter))
    {
        mathCode.remove(0, 1);
        mathCode.chop(1);

        if (mathCode[0] == QChar(6))
            mathCode.remove(0, 1);

        return std::make_pair(mathCode, Cantor::LatexRenderer::InlineEquation);
    }
    else if (mathCode.startsWith(QString::fromUtf8("\\begin{")) && mathCode.endsWith(QLatin1Char('}')))
    {
        if (mathCode[1] == QChar(6))
            mathCode.remove(1, 1);

        return std::make_pair(mathCode, Cantor::LatexRenderer::CustomEquation);
    }
    else
        return std::make_pair(QString(), Cantor::LatexRenderer::InlineEquation);
}

void MarkdownEntry::setRenderedMath(int jobId, const QTextImageFormat& format, const QUrl& internal, const QImage& image)
{
    if ((int)foundMath.size() < jobId)
        return;

    const auto& iter = foundMath.begin() + jobId-1;

    QTextCursor cursor = findMath(jobId);

    const QString delimiter = format.property(Cantor::Renderer::Delimiter).toString();
    QString searchText = delimiter + format.property(Cantor::Renderer::Code).toString() + delimiter;

    Cantor::LatexRenderer::EquationType type
        = (Cantor::LatexRenderer::EquationType)format.intProperty(Cantor::Renderer::CantorFormula);

    // From findMath we will be first symbol of math expression
    // So in order to select all symbols of the expression, we need to go to previous symbol first
    // But it working strange sometimes: some times we need to go to previous character, sometimes not
    // So the code tests that we on '$' symbol and if it isn't true, then we revert back
    cursor.movePosition(QTextCursor::PreviousCharacter);
    bool withDollarDelimiter = type == Cantor::LatexRenderer::InlineEquation || type == Cantor::LatexRenderer::FullEquation;
    if (withDollarDelimiter && m_textItem->document()->characterAt(cursor.position()) != QLatin1Char('$'))
        cursor.movePosition(QTextCursor::NextCharacter);
    else if (type == Cantor::LatexRenderer::CustomEquation && m_textItem->document()->characterAt(cursor.position()) != QLatin1Char('\\') )
        cursor.movePosition(QTextCursor::NextCharacter);

    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, searchText.size());

    if (!cursor.isNull())
    {
        m_textItem->document()->addResource(QTextDocument::ImageResource, internal, QVariant(image));

        // Don't add new line for $$...$$ on document's begin and end
        // And if we in block, which haven't non-space characters except out math expression
        // In another sitation, Cantor will move rendered image into another QTextBlock
        QTextCursor prevSymCursor = m_textItem->document()->find(QRegularExpression(QStringLiteral("[^\\s]")),
                                                                 cursor, QTextDocument::FindBackward);
        if (type == Cantor::LatexRenderer::FullEquation
            && cursor.selectionStart() != 0
            && prevSymCursor.block() == cursor.block()
        )
        {
            cursor.insertBlock();

            cursor.setPosition(prevSymCursor.position()+2, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }

        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);

        bool atDocEnd = cursor.position() == m_textItem->document()->characterCount()-1;
        QTextCursor nextSymCursor = m_textItem->document()->find(QRegularExpression(QStringLiteral("[^\\s]")), cursor);
        if (type == Cantor::LatexRenderer::FullEquation && !atDocEnd && nextSymCursor.block() == cursor.block())
        {
            cursor.setPosition(nextSymCursor.position()-1, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.insertBlock();
        }

        // Set that the formulas is rendered
        iter->second = true;

        m_textItem->document()->clearUndoRedoStacks();
    }
}

QTextCursor MarkdownEntry::findMath(int id)
{
    QTextCursor cursor(m_textItem->document());
    do
    {
        QTextCharFormat format = cursor.charFormat();
        if (format.intProperty(JobProperty) == id)
            break;
    }
    while (cursor.movePosition(QTextCursor::NextCharacter));

    return cursor;
}

void MarkdownEntry::markUpMath()
{
    QTextCursor cursor(m_textItem->document());
    for (int i = 0; i < (int)foundMath.size(); i++)
    {
        if (foundMath[i].second)
            continue;

        QString searchText = foundMath[i].first;
        searchText.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));

        cursor = m_textItem->document()->find(searchText, cursor);

        // Mark up founded math code
        QTextCharFormat format = cursor.charFormat();
        // Use index+1 in math array as property tag
        format.setProperty(JobProperty, i+1);

        // We found the math expression, so remove 'marker' (ACII symbol 'Acknowledgement')
        // The marker have been placed after "$" or "$$"
        // We remove the marker, only if it presents
        QString codeWithoutMarker = foundMath[i].first;
        if (searchText.startsWith(QLatin1String("$$")))
        {
            if (codeWithoutMarker[2] == QChar(6))
                codeWithoutMarker.remove(2, 1);
        }
        else if (searchText.startsWith(QLatin1String("$")))
        {
            if (codeWithoutMarker[1] == QChar(6))
                codeWithoutMarker.remove(1, 1);
        }
        else if (searchText.startsWith(QLatin1String("\\")))
        {
            if (codeWithoutMarker[1] == QChar(6))
                codeWithoutMarker.remove(1, 1);
        }
        cursor.insertText(codeWithoutMarker, format);
    }
}

void MarkdownEntry::insertImage()
{
    KConfigGroup conf(KSharedConfig::openConfig(), "MarkdownEntry");
    QString dir = conf.readEntry("LastImageDir", "");

    QString formats;
    for (const QByteArray& format : QImageReader::supportedImageFormats()) {
        QString f = QLatin1String("*.") + QLatin1String(format.constData());
        formats.isEmpty() ? formats += f : formats += QLatin1Char(' ') + f;
    }

    const QString& path = QFileDialog::getOpenFileName(worksheet()->worksheetView(),
                                                       i18n("Open image file"),
                                                       dir,
                                                       i18n("Images (%1)",
                                                       formats));
    if (path.isEmpty())
        return; //cancel was clicked in the file-dialog

    //save the last used directory, if changed
    const int pos = path.lastIndexOf(QLatin1String("/"));
    if (pos != -1) {
        const QString& newDir = path.left(pos);
        if (newDir != dir)
            conf.writeEntry(QLatin1String("LastImageDir"), newDir);
    }

    QImageReader reader(path);
    const QImage& img = reader.read();
    if (!img.isNull())
    {
        const QString& name = QFileInfo(path).fileName();
        addImageAttachment(name, img);
    }
    else
        KMessageBox::error(worksheetView(),
                           i18n("Failed to read the image \"%1\". Error \"%2\"", path, reader.errorString()),
                           i18n("Cantor"));
}

void MarkdownEntry::clearAttachments()
{
    for (auto& attachment: attachedImages)
    {
        const QUrl& url = attachment.first;
        m_textItem->document()->addResource(QTextDocument::ImageResource, url, QVariant());
    }
    attachedImages.clear();
    animateSizeChange();
}

void MarkdownEntry::enterEditMode()
{
    setPlainText(plain);
    m_textItem->textCursor().clearSelection();
    rendered = false;
}

QString MarkdownEntry::plainText() const
{
    return m_textItem->toPlainText();
}

void MarkdownEntry::addImageAttachment(const QString& name, const QImage& image)
{
    QUrl url;
    url.setScheme(QLatin1String("attachment"));
    url.setPath(name);

    attachedImages.push_back(std::make_pair(url, QLatin1String("image/png")));
    m_textItem->document()->addResource(QTextDocument::ImageResource, url, QVariant(image));

    QTextCursor cursor = m_textItem->textCursor();
    cursor.insertText(QString::fromLatin1("![%1](attachment:%1)").arg(name));

    animateSizeChange();
}
