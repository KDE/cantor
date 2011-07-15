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
#include <kmenu.h>
#include <kicon.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kaction.h>


TextEntry::TextEntry(QTextCursor position, Worksheet* parent ) : WorksheetEntry( position, parent )
{
    QTextFrameFormat frameFormat = m_frame->frameFormat();
    frameFormat.setPadding(10);
    m_frame->setFrameFormat(frameFormat);
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

bool TextEntry::worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    KMenu* defaultMenu = new KMenu(m_worksheet);

    defaultMenu->addAction(KStandardAction::cut(m_worksheet));
    defaultMenu->addAction(KStandardAction::copy(m_worksheet));
    defaultMenu->addAction(KStandardAction::paste(m_worksheet));
    defaultMenu->addSeparator();

    if(!m_worksheet->isRunning())
	defaultMenu->addAction(KIcon("system-run"),i18n("Evaluate Worksheet"),m_worksheet,SLOT(evaluate()),0);
    else
	defaultMenu->addAction(KIcon("process-stop"),i18n("Interrupt"),m_worksheet,SLOT(interrupt()),0);
    defaultMenu->addSeparator();

    defaultMenu->addAction(KIcon("edit-delete"),i18n("Remove Entry"), m_worksheet, SLOT(removeCurrentEntry()));

    createSubMenuInsert(defaultMenu);

    defaultMenu->popup(event->globalPos());
	
    return true;
}


void TextEntry::setContent(const QString& content)
{
    firstValidCursorPosition().insertText(content);
}

void TextEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);
    if(content.firstChildElement("body").isNull())
	return;

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
