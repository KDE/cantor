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

#include <config-cantor.h>

#ifdef Discount_FOUND
extern "C" {
#include <mkdio.h>
}
#endif

#include <QDebug>

MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : WorksheetEntry(worksheet), m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction)), rendered(false)
{
    m_textItem->enableRichText(false);
    m_textItem->setOpenExternalLinks(true);
    m_textItem->installEventFilter(this);
    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &MarkdownEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &MarkdownEntry::moveToNextEntry);
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
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
    if(rendered)
        setRenderedHtml(html);
    else
        setPlainText(plain);
}

void MarkdownEntry::setContentFromJupyter(const QJsonObject& cell)
{
    //Jupyter TODO: handle missing key, like 'source', check that array, and string data inside
    const QJsonArray& sources = cell.value(QLatin1String("source")).toArray();
    QString code;
    for (const QJsonValue& line : sources)
        code += line.toString();

    setPlainText(adaptJupyterMarkdown(code));
}

QDomElement MarkdownEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

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
    return el;
}

QString MarkdownEntry::toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();

    if(!rendered)
        plain = m_textItem->toPlainText();

    QString text = QString(plain);

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
        plain = m_textItem->toPlainText();
        rendered = renderMarkdown(plain);
    }

    if (rendered)
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
    }

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
            if(mouseEvent->button() == Qt::LeftButton && rendered)
            {
                setPlainText(plain);
                m_textItem->setCursorPosition(mouseEvent->pos());
                m_textItem->textCursor().clearSelection();
                rendered = false;
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

QTextCursor MarkdownEntry::findLatexCode(const QTextCursor& cursor) const
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

enum class ParserState {Text, DoubleQuotes, SingleQuotes, CodeQuotes, SingleDollar, DoubleDollar};
QString MarkdownEntry::adaptJupyterMarkdown(const QString& markdown)
{
    static const QLatin1Char DOUBLE_QUOTE('"');
    static const QLatin1Char SINGLE_QUOTE('\'');
    static const QLatin1Char CODE_QUOTE('`');
    static const QLatin1Char DOLLAR('$');
    static const QLatin1Char ESCAPER('\\');

    QString buf;
    QChar prev[2];
    ParserState state = ParserState::Text;
    QString out;
    for (const QChar& sym : markdown)
    {
        const bool isEscaping = prev[0] == ESCAPER;
        switch (state)
        {
            case ParserState::Text:
            {
                if (sym == CODE_QUOTE && !isEscaping)
                    state = ParserState::CodeQuotes;
                else if (sym == DOUBLE_QUOTE && !isEscaping)
                    state = ParserState::DoubleQuotes;
                else if (sym == SINGLE_QUOTE && !isEscaping)
                    state = ParserState::SingleQuotes;

                const bool isDoubleEscaping = isEscaping && prev[1] == ESCAPER;
                if (sym == DOLLAR && !isDoubleEscaping)
                {
                    state = ParserState::SingleDollar;
                    // no write to out variable
                    break;
                }

                out += sym;
            }
                break;

            case ParserState::SingleQuotes:
                out += sym;

                if (sym == SINGLE_QUOTE && !isEscaping)
                    state = ParserState::Text;
                break;

            case ParserState::DoubleQuotes:
                out += sym;

                if (sym == DOUBLE_QUOTE && !isEscaping)
                    state = ParserState::Text;
                break;

            case ParserState::CodeQuotes:
                out += sym;

                if (sym == CODE_QUOTE && !isEscaping)
                    state = ParserState::Text;
                break;

            case ParserState::SingleDollar:
                if (sym == DOLLAR)
                {
                    if (isEscaping)
                        buf += sym;
                    else
                    {
                        // So we have double dollars ($$) and we need go to double dollar state
                        if (buf.isEmpty())
                        {
                            out += DOLLAR + DOLLAR;
                            state = ParserState::DoubleDollar;
                        }
                        else
                        {
                            // Main purpose of this code
                            // $...$ -> $$...$$
                            // because Cantor don't support only $$...$$
                            out += DOLLAR + DOLLAR + buf + DOLLAR + DOLLAR;
                            buf.clear();
                            state = ParserState::Text;
                        }
                    }
                }
                else
                    buf += sym;
                break;

            case ParserState::DoubleDollar:
                out += sym;
                if (sym == DOLLAR && prev[0] == DOLLAR && prev[1] != ESCAPER)
                    state = ParserState::Text;
                break;
        }

        // Shift previous symbols
        prev[1] = prev[0];
        prev[0] = sym;
    }
    out.replace(QLatin1String("\\\\$"), QLatin1String("$"));
    return out;
}
