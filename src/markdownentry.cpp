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

#include "config-cantor.h"

#ifdef discount_FOUND
extern "C" {
#include <mkdio.h>
}
#endif

#include <QDebug>

MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : TextEntry(worksheet), dirty(false), evalJustNow(false)
{
	m_textItem->installEventFilter(this);
}

MarkdownEntry::~MarkdownEntry()
{
}

void MarkdownEntry::setContent(const QString& content)
{
	dirty = true;
	plain = content;
	TextEntry::setContent(content);
}

void MarkdownEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);

	dirty = true;
    plain = content.text();
    m_textItem->setPlainText(plain);
    qDebug() << plain;
}

QDomElement MarkdownEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    qDebug() << plain;
    QDomElement el = doc.createElement(QLatin1String("Markdown"));
	QDomText text=doc.createTextNode(plain);
    el.appendChild(text);
    return el;
}

bool MarkdownEntry::evaluate(EvaluationOption evalOp)
{
#ifdef discount_FOUND
	if(m_textItem->hasFocus()) // text in the entry may be edited
		plain = m_textItem->toPlainText();

    /*
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QTemporaryFile* markdownQFile = new QTemporaryFile(tempDir + QLatin1String("/cantor_md-XXXXXX.md"));
    markdownQFile->open(QIODevice::ReadOnly);
    FILE* mdFile = fdopen(markdownQFile->handle(), "rb");
    if(!mdFile)
    {
        qDebug()<<"Failed to open the markdown temporary file";
        return TextEntry::evaluate(evalOp);
    }
    */

	// convert markdown to html
    QByteArray mdCharArray = plain.toUtf8();
    MMIOT* mdHandle = mkd_string(mdCharArray.data(), mdCharArray.size()+1, 0); // get the size of the string in byte
    if(!mkd_compile(mdHandle, MKD_NOSUPERSCRIPT | MKD_FENCEDCODE | MKD_GITHUBTAGS))
    {
        qDebug()<<"Failed to compile the markdown document";
        return TextEntry::evaluate(evalOp);
    }
    char *htmlDocument;
    int htmlSize = mkd_document(mdHandle, &htmlDocument);
    html = QString::fromUtf8(htmlDocument, htmlSize);

	m_textItem->setHtml(html);
	dirty = false;
	evalJustNow = true;
#endif
	return TextEntry::evaluate(evalOp);
}

bool MarkdownEntry::eventFilter(QObject* object, QEvent* event)
{
	if(object == m_textItem)
	{
		if(event->type() == QEvent::FocusIn)
		{
			QString plainHtml = QLatin1String("<p>") + plain + QLatin1String("</p>"); // clear the style, such as font
			plainHtml.replace(QLatin1String("\n"), QLatin1String("<br>"));
			m_textItem->setHtml(plainHtml); 
		}
		else if(event->type() == QEvent::FocusOut)
		{
			if(evalJustNow)
			{
				evalJustNow = false;
				return false;
			}

			if(!dirty && plain.compare(m_textItem->toPlainText()) == 0)
			{
				m_textItem->setHtml(html);
				TextEntry::evaluate();
			}
			else
			{
				dirty = true;
				plain = m_textItem->toPlainText();
			}
		}
	}
	return false;
}
