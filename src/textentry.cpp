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
    Copyright (C) 2010 Raffaele De Feo <alberthilbert@gmail.com>
 */

#include "textentry.h"
#include "worksheetentry.h"
#include "worksheet.h"
#include "lib/defaulthighlighter.h"

#include <QEvent>
#include <QKeyEvent>
#include <QTextDocumentFragment>

#include <kdebug.h>
#include <kzip.h>


TextEntry::TextEntry(QTextCursor position, Worksheet* parent ) : WorksheetEntry( position, parent )
{
    QTextCharFormat format = firstCursorPosition().blockCharFormat();
    format.setProperty(Cantor::DefaultHighlighter::BlockTypeProperty, Cantor::DefaultHighlighter::NoHighlightBlock);
    firstCursorPosition().setBlockCharFormat(format);
}

TextEntry::~TextEntry()
{

}

int TextEntry::type()
{
    return Type;
}

QTextCursor TextEntry::firstValidCursorPosition()
{
    return firstCursorPosition();
}

QTextCursor TextEntry::lastValidCursorPosition()
{
    return lastCursorPosition();
}

QTextCursor TextEntry::closestValidCursor(const QTextCursor& cursor)
{
    return QTextCursor(cursor);
}

bool TextEntry::isValidCursor(const QTextCursor& cursor)
{
    int pos = cursor.position();
    return (firstValidPosition() <= pos && pos <= lastValidPosition());
}

bool TextEntry::isEmpty()
{
    QTextCursor cursor = firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    return cursor.selection().isEmpty();
}

bool TextEntry::acceptRichText()
{
    return true;
}

bool TextEntry::acceptsDrop(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);

    return true;
}

void TextEntry::setContent(const QString& content)
{
    firstValidCursorPosition().insertText(content);
}

void TextEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);

    QDomDocument doc = QDomDocument();
    QDomNode n = doc.importNode(content.firstChildElement("body"), true);
    doc.appendChild(n);
    QString html = doc.toString();
    kDebug() << html;
    firstValidCursorPosition().insertHtml(html);
}

QDomElement TextEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    QTextCursor cursor = firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    const QString& html = cursor.selection().toHtml();
    kDebug() << html;
    QDomElement el = doc.createElement("Text");
    QDomDocument myDoc = QDomDocument();
    myDoc.setContent(html);
    el.appendChild(myDoc.documentElement().firstChildElement("body"));
    return el;
}

QString TextEntry::toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);

    if (commentStartingSeq.isEmpty())
        return QString();
    QTextCursor cursor = firstValidCursorPosition();
    cursor.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
    QString text = cursor.selection().toPlainText();
    if (!commentEndingSeq.isEmpty())
        return commentStartingSeq + text + commentEndingSeq + "\n";
    return commentStartingSeq + text.replace("\n", "\n" + commentStartingSeq) + "\n";
}

void TextEntry::interruptEvaluation()
{

}

bool TextEntry::evaluate(bool current)
{
    Q_UNUSED(current);
    return true;
}

void TextEntry::update()
{

}
