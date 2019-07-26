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
    Copyright (C) 2018 Yifei Wu <kqwyfg@gmail.com>
 */

#include "markdownentry.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QImage>
#include <QImageReader>
#include <QBuffer>
#include <KLocalizedString>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <KMessageBox>

#include "jupyterutils.h"
#include "mathrender.h"
#include <config-cantor.h>

#ifdef Discount_FOUND
extern "C" {
#include <mkdio.h>
}
#endif


MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : WorksheetEntry(worksheet), m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction)), rendered(false)
{
    m_textItem->enableRichText(false);
    m_textItem->setOpenExternalLinks(true);
    m_textItem->installEventFilter(this);
    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &MarkdownEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &MarkdownEntry::moveToNextEntry);
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
}

void MarkdownEntry::populateMenu(QMenu* menu, QPointF pos)
{
    if (!rendered)
        menu->addAction(i18n("Insert Image Attachment"), this, &MarkdownEntry::insertImage);
    if (attachedImages.size() != 0)
        menu->addAction(i18n("Clear Attachments"), this, &MarkdownEntry::clearAttachments);
    WorksheetEntry::populateMenu(menu, pos);
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

                    // Get uuid by removing 'cantor_' and '.pdf' extention
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
                else
                    renderMathExpression(i+1, mathCode);
            }
        }
    }
}

void MarkdownEntry::setContentFromJupyter(const QJsonObject& cell)
{
    if (!JupyterUtils::isMarkdownCell(cell))
        return;

    // https://nbformat.readthedocs.io/en/latest/format_description.html#cell-metadata
    // There isn't Jupyter metadata for markdown cells, which could be handled by Cantor
    // So we just store it
    setJupyterMetadata(JupyterUtils::getMetadata(cell));

    const QJsonObject attachments = cell.value(QLatin1String("attachments")).toObject();
    for (const QString& key : attachments.keys())
    {
        const QJsonValue& attachment = attachments.value(key);
        const QString& mimeKey = JupyterUtils::firstImageKey(attachment);
        if (!mimeKey.isEmpty())
        {
            const QImage& image = JupyterUtils::loadImage(attachment, mimeKey);

            QUrl resourceUrl;
            resourceUrl.setUrl(QLatin1String("attachment:")+key);
            attachedImages.push_back(std::make_pair(resourceUrl, mimeKey));
            m_textItem->document()->addResource(QTextDocument::ImageResource, resourceUrl, QVariant(image));
        }
    }

    setPlainText(JupyterUtils::getSource(cell));
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
                if (format.hasProperty(EpsRenderer::CantorFormula))
                {
                    const QString& latex = format.property(EpsRenderer::Code).toString();
                    const QString& delimiter = format.property(EpsRenderer::Delimiter).toString();
                    const QString& code = delimiter + latex + delimiter;
                    if (code == data.first)
                    {
                        const QUrl& url = QUrl::fromLocalFile(format.property(EpsRenderer::ImagePath).toString());
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
        attachments.insert(attachmentKey, JupyterUtils::packMimeBundle(image, key));
    }
    if (!attachments.isEmpty())
        entry.insert(QLatin1String("attachments"), attachments);

    JupyterUtils::setSource(entry, plain);

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
    }

    if (worksheet()->embeddedMathEnabled())
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
        if (format.hasProperty(EpsRenderer::CantorFormula))
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

void MarkdownEntry::layOutForWidth(qreal w, bool force)
{
    if (size().width() == w && !force)
        return;

    m_textItem->setGeometry(0, 0, w);
    setSize(QSizeF(m_textItem->width(), m_textItem->height() + VerticalMargin));
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
        renderMathExpression(i+1, foundMath[i].first);
}

void MarkdownEntry::handleMathRender(QSharedPointer<MathRenderResult> result)
{
    if (!result->successfull)
    {
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
    else
        return std::make_pair(QString(), Cantor::LatexRenderer::InlineEquation);
}

void MarkdownEntry::setRenderedMath(int jobId, const QTextImageFormat& format, const QUrl& internal, const QImage& image)
{
    if ((int)foundMath.size() < jobId)
        return;

    const auto& iter = foundMath.begin() + jobId-1;

    QTextCursor cursor = findMath(jobId);

    const QString delimiter = format.property(EpsRenderer::Delimiter).toString();
    QString searchText = delimiter + format.property(EpsRenderer::Code).toString() + delimiter;
    searchText.replace(QRegExp(QLatin1String("\\s+")), QLatin1String(" "));

    // From findMath we will be first symbol of math expression
    // So in order to select all symbols of the expression, we need to go to previous symbol first
    cursor.movePosition(QTextCursor::PreviousCharacter);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, searchText.size());

    if (!cursor.isNull())
    {
        m_textItem->document()->addResource(QTextDocument::ImageResource, internal, QVariant(image));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);

        // Set that the formulas is rendered
        iter->second = true;
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
        searchText.replace(QRegExp(QLatin1String("\\s+")), QLatin1String(" "));

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
        cursor.insertText(codeWithoutMarker, format);
    }
}

void MarkdownEntry::insertImage()
{
    const QString& filename = QFileDialog::getOpenFileName(worksheet()->worksheetView(), i18n("Choose Image"), QString(), i18n("Images (*.png *.bmp *.jpg *.svg)"));

    if (!filename.isEmpty())
    {
        QImageReader reader(filename);
        const QImage img = reader.read();
        if (!img.isNull())
        {
            const QString& name = QFileInfo(filename).fileName();

            QUrl url;
            url.setScheme(QLatin1String("attachment"));
            url.setPath(name);

            attachedImages.push_back(std::make_pair(url, QLatin1String("image/png")));
            m_textItem->document()->addResource(QTextDocument::ImageResource, url, QVariant(img));

            QTextCursor cursor = m_textItem->textCursor();
            cursor.insertText(QString::fromLatin1("![%1](attachment:%1)").arg(name));

            animateSizeChange();
        }
        else
            KMessageBox::error(worksheetView(), i18n("Cantor failed to read image with error \"%1\"", reader.errorString()), i18n("Cantor"));
    }
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
