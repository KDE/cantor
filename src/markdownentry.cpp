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
    bool imageSelected = false;
    QTextCursor cursor = m_textItem->textCursor();
    const QChar repl = QChar::ObjectReplacementCharacter;
    if (cursor.hasSelection()) {
        QString selection = m_textItem->textCursor().selectedText();
        imageSelected = selection.contains(repl);
    } else {
        // we need to try both the current cursor and the one after the that
        cursor = m_textItem->cursorForPosition(pos);
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
        menu->addAction(i18n("Show LaTeX code"), this, &MarkdownEntry::resolveImagesAtCursor);
        menu->addSeparator();
    }
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
    Q_UNUSED(file);

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
}

void MarkdownEntry::setContentFromJupyter(const QJsonObject& cell)
{
    if (!JupyterUtils::isMarkdownCell(cell))
        return;

    // https://nbformat.readthedocs.io/en/latest/format_description.html#cell-metadata
    // There isn't Jupyter metadata for markdown cells, which could be handled by Cantor

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
    Q_UNUSED(archive)

    if(!rendered)
        plain = m_textItem->toPlainText();

    QDomElement el = doc.createElement(QLatin1String("Markdown"));
    el.setAttribute(QLatin1String("rendered"), (int)rendered);

    QDomElement plainEl = doc.createElement(QLatin1String("Plain"));
    plainEl.appendChild(doc.createTextNode(plain));
    el.appendChild(plainEl);
    if(rendered)
    {
        QDomElement htmlEl = doc.createElement(QLatin1String("HTML"));
        htmlEl.appendChild(doc.createTextNode(html));
        el.appendChild(htmlEl);
    }

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

        /*
        QString attachmentKey = url.toString().remove(QLatin1String("attachment:"));
        attachments.insert(attachmentKey, JupyterUtils::packMimeBundle(image, key));
        */
    }

    return el;
}

QJsonValue MarkdownEntry::toJupyterJson()
{
    QJsonObject entry;

    entry.insert(QLatin1String("cell_type"), QLatin1String("markdown"));

    entry.insert(QLatin1String("metadata"), QJsonObject());

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
        if (m_textItem->toPlainText() == plain)
        {
            setRenderedHtml(html);
            rendered = true;
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
    for (const QString& latex : latexUnits)
    {
        foundMath.push_back(std::make_pair(latex, false));
        html.replace(latex, latex.toHtmlEscaped());
    }
    qDebug() << foundMath;

    mkd_cleanup(mdHandle);

    setRenderedHtml(html);
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

void MarkdownEntry::resolveImagesAtCursor()
{
    QTextCursor cursor = m_textItem->textCursor();
    if (!cursor.hasSelection())
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    cursor.insertText(m_textItem->resolveImages(cursor));
}

void MarkdownEntry::renderMath()
{
    QTextCursor cursor(m_textItem->document());
    cursor.movePosition(QTextCursor::Start);
    for (std::pair<QString, bool> pair : foundMath)
    {
        static const QLatin1String inlineDelimiter("$");
        static const QLatin1String displayedDelimiter("$$");
        // Jupyter TODO: what about \( \) and \[ \]? Supports or not?

        QString searchText = pair.first;
        searchText.replace(QRegExp(QLatin1String("\\s+")), QLatin1String(" "));

        QTextCursor found = m_textItem->document()->find(searchText, cursor);
        if (found.isNull())
            continue;
        cursor = found;

        QString latexCode = cursor.selectedText();
        latexCode.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        latexCode.replace(QChar::LineSeparator, QLatin1Char('\n'));
        qDebug()<<"found latex: " << latexCode;

        MathRenderer* renderer = worksheet()->mathRenderer();
        if (latexCode.startsWith(displayedDelimiter) && latexCode.endsWith(displayedDelimiter))
        {
            latexCode.remove(0, 2);
            latexCode.chop(2);

            renderer->renderExpression(latexCode, Cantor::LatexRenderer::FullEquation, this, SLOT(handleMathRender(QSharedPointer<MathRenderResult>)));
        }
        else if (latexCode.startsWith(inlineDelimiter) && latexCode.endsWith(inlineDelimiter))
        {
            latexCode.remove(0, 1);
            latexCode.chop(1);

            renderer->renderExpression(latexCode, Cantor::LatexRenderer::InlineEquation, this, SLOT(handleMathRender(QSharedPointer<MathRenderResult>)));
        }
    }
}

void MarkdownEntry::handleMathRender(QSharedPointer<MathRenderResult> result)
{
    if (!result->successfull)
    {
        qDebug() << "MarkdownEntry: math render failed with message" << result->errorMessage;
        return;
    }

    const QString& code = result->renderedMath.property(EpsRenderer::Code).toString();
    const QString& delimiter = result->renderedMath.property(EpsRenderer::Delimiter).toString();
    QString searchText = delimiter + code + delimiter;
    searchText.replace(QRegExp(QLatin1String("\\s")), QLatin1String(" "));

    QTextCursor cursor = m_textItem->document()->find(searchText);
    if (!cursor.isNull())
    {
        m_textItem->document()->addResource(QTextDocument::ImageResource, result->uniqueUrl, QVariant(result->image));
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), result->renderedMath);
    }
}
