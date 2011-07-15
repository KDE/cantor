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
    Copyright (C) 2011 Martin Kuettler <martinkuettler@gmail.com>
 */

#include "pagebreakentry.h"
#include "worksheet.h"

#include <kcolorscheme.h>
#include <kmenu.h>
#include <kicon.h>
#include <klocale.h>

PageBreakEntry::PageBreakEntry(QTextCursor position, Worksheet* parent) : 
    WorksheetEntry( position, parent )
{
    QTextFrameFormat frameFormat = m_frame->frameFormat();

    frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    frameFormat.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);

    m_frame->setFrameFormat(frameFormat);

    // do I need to call this?
    update();
}

PageBreakEntry::~PageBreakEntry()
{

}

int PageBreakEntry::type()
{
    return Type;
}

bool PageBreakEntry::isEmpty()
{
    return true;
}


QTextCursor PageBreakEntry::closestValidCursor(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    return firstValidCursorPosition();
}

QTextCursor PageBreakEntry::firstValidCursorPosition()
{
    return m_frame->lastCursorPosition();
}

QTextCursor PageBreakEntry::lastValidCursorPosition()
{
    return m_frame->lastCursorPosition();
}

bool PageBreakEntry::isValidCursor(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    return false;
}

bool PageBreakEntry::worksheetKeyPressEvent(QKeyEvent* event, const QTextCursor& cursor)
{
    if (WorksheetEntry::worksheetKeyPressEvent(event, cursor))
	return true;
    
    // Are there any other keys we should allow here?
    return true;
}

bool PageBreakEntry::worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    KMenu* defaultMenu = new KMenu(m_worksheet);

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


bool PageBreakEntry::acceptRichText()
{
    return false;
}

bool PageBreakEntry::acceptsDrop(const QTextCursor& cursor)
{
    Q_UNUSED(cursor);
    return false;
}

void PageBreakEntry::setContent(const QString& content)
{
    Q_UNUSED(content);
    return;
}

void PageBreakEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(content);
    Q_UNUSED(file);
    return;
}

QDomElement PageBreakEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);
    
    QDomElement pgbrk = doc.createElement("PageBreak");
    return pgbrk;
}

QString PageBreakEntry::toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq)
{
    Q_UNUSED(commandSep);
    
    return commentStartingSeq + "page break" + commentEndingSeq;
    
}


void PageBreakEntry::interruptEvaluation()
{
    return;
}

bool PageBreakEntry::evaluate(bool current)
{
    Q_UNUSED(current);
    return true;
}

void PageBreakEntry::update()
{
    QTextCursor cursor(m_frame->firstCursorPosition());
    cursor.setPosition(m_frame->lastPosition(), QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    if (not m_worksheet->isPrinting())
    {
	QTextBlockFormat block(cursor.blockFormat());
	block.setAlignment(Qt::AlignCenter);
	cursor.setBlockFormat(block);
	KColorScheme color = KColorScheme( QPalette::Normal, KColorScheme::View);
	QTextCharFormat cformat(cursor.charFormat());
	cformat.setForeground(color.foreground(KColorScheme::InactiveText));

	cursor.insertText("--- Page Break ---", cformat);
    }
}
