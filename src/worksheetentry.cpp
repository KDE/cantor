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
    Copyright (C) 2010 Raffaele De Feo <alberthilbert@gmail.com>
 */

#include "worksheetentry.h"
#include "worksheet.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTextFrame>

#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>

WorksheetEntry::WorksheetEntry(QTextCursor position, Worksheet* parent ) : QObject( parent )
{
    m_worksheet = parent;

    QTextFrameFormat frameFormat;
    frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    frameFormat.setBorder(1);
    frameFormat.setLeftMargin(6);
    frameFormat.setRightMargin(6);

    connect(this, SIGNAL(destroyed(QObject*)), m_worksheet, SLOT(removeEntry(QObject*)));
    connect(this, SIGNAL(leftmostValidPositionReached()), m_worksheet, SLOT(moveToPreviousEntry()));
    connect(this, SIGNAL(rightmostValidPositionReached()), m_worksheet, SLOT(moveToNextEntry()));
    connect(this, SIGNAL(topmostValidLineReached()), m_worksheet, SLOT(moveToPreviousEntry()));
    connect(this, SIGNAL(bottommostValidLineReached()), m_worksheet, SLOT(moveToNextEntry()));

    m_frame = position.insertFrame(frameFormat);
}

WorksheetEntry::~WorksheetEntry()
{

}

int WorksheetEntry::type()
{
    return Type;
}

void WorksheetEntry::setActive(bool active, bool moveCursor)
{
    if (active && moveCursor && !isValidCursor(m_worksheet->textCursor()))
        m_worksheet->setTextCursor(firstValidCursorPosition());
}

QTextCursor WorksheetEntry::firstCursorPosition()
{
    if(m_frame)
        return m_frame->firstCursorPosition();
    return QTextCursor();
}

QTextCursor WorksheetEntry::lastCursorPosition()
{
    if(m_frame)
        return m_frame->lastCursorPosition();
    return QTextCursor();
}

int WorksheetEntry::firstPosition()
{
    if(m_frame)
        return m_frame->firstCursorPosition().position();
    return -1;
}

int WorksheetEntry::lastPosition()
{
    if(m_frame)
        return m_frame->lastCursorPosition().position();
    return -1;
}

bool WorksheetEntry::contains(const QTextCursor& cursor)
{
    if(!m_frame)
        return false;

    if(cursor.position()>=firstPosition() && cursor.position()<=lastPosition())
        return true;
    return false;
}

int WorksheetEntry::firstValidPosition()
{
    return firstValidCursorPosition().position();
}

int WorksheetEntry::lastValidPosition()
{
    return lastValidCursorPosition().position();
}

bool WorksheetEntry::worksheetShortcutOverrideEvent(QKeyEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(cursor);

    //tell Worksheet to ignore the following shortcuts,
    //so they can be used as a Shortcut for a KAction:
    //Shift+Return
    //Shift+Delete

    int key = event->key();

    int modifiers = event->modifiers();
    if (modifiers == Qt::ShiftModifier && (key == Qt::Key_Return || key == Qt::Key_Enter))
        return true;
    else if (modifiers == Qt::ShiftModifier && key == Qt::Key_Delete)
        return true;

    return false;
}

bool WorksheetEntry::worksheetKeyPressEvent(QKeyEvent* event, const QTextCursor& cursor)
{
    int key = event->key();
    int position = cursor.position();

    if (key == Qt::Key_Left && position == firstValidPosition())
    {
        emit leftmostValidPositionReached();
        kDebug()<<"Reached leftmost valid position";
        return true;

    }
    else if (key == Qt::Key_Right && position == lastValidPosition())
    {
        emit rightmostValidPositionReached();
        kDebug()<<"Reached rightmost valid position";
        return true;

    }
    else if (key == Qt::Key_Up)
    {
        QTextCursor c = QTextCursor(cursor);
        c.setPosition(firstValidPosition(), QTextCursor::KeepAnchor);
        QString txt = c.selectedText();

        if(!(txt.contains(QChar::ParagraphSeparator)||
            txt.contains(QChar::LineSeparator)||
            txt.contains('\n'))) //there's still a newline above the cursor, so move only one line up
        {
            emit topmostValidLineReached();
            kDebug()<<"Reached topmost valid line";
            return true;
        }
    }
    else if (key == Qt::Key_Down )
    {
        QTextCursor c = QTextCursor(cursor);
        c.setPosition(lastValidPosition(), QTextCursor::KeepAnchor);
        QString txt = c.selectedText();

        if(!(txt.contains(QChar::ParagraphSeparator)||
            txt.contains(QChar::LineSeparator)||
            txt.contains('\n'))) //there's still a newline under the cursor, so move only one line down
        {
            emit bottommostValidLineReached();
            kDebug()<<"Reached bottommost valid line";
            return true;
        }
    }
    return false;
}

bool WorksheetEntry::worksheetMousePressEvent(QMouseEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(event);
    Q_UNUSED(cursor);

    return false;
}

bool WorksheetEntry::worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(event);
    Q_UNUSED(cursor);

    return false;
}

bool WorksheetEntry::worksheetMouseDoubleClickEvent(QMouseEvent* event, const QTextCursor& cursor)
{
    Q_UNUSED(event);
    Q_UNUSED(cursor);

    return false;
}

void WorksheetEntry::checkForSanity()
{

}

void WorksheetEntry::showCompletion()
{

}

void WorksheetEntry::createSubMenuInsert(KMenu* menu)
{
    KMenu* subMenuInsert = new KMenu(menu);
    KMenu* subMenuInsertBefore = new KMenu(menu);

    subMenuInsert->addAction(i18n("Command Entry"), m_worksheet, SLOT(insertCommandEntry()));
    subMenuInsert->addAction(i18n("Text Entry"), m_worksheet, SLOT(insertTextEntry()));
    subMenuInsert->addAction(i18n("Image Entry"), m_worksheet, SLOT(insertImageEntry()));
    subMenuInsert->addAction(i18n("Page Break"), m_worksheet, SLOT(insertPageBreakEntry()));

    subMenuInsertBefore->addAction(i18n("Command Entry"), m_worksheet, SLOT(insertCommandEntryBefore()));
    subMenuInsertBefore->addAction(i18n("Text Entry"), m_worksheet, SLOT(insertTextEntryBefore()));
    subMenuInsertBefore->addAction(i18n("Image Entry"), m_worksheet, SLOT(insertImageEntryBefore()));
    subMenuInsertBefore->addAction(i18n("Page Break"), m_worksheet, SLOT(insertPageBreakEntryBefore()));

    subMenuInsert->setTitle(i18n("Insert Entry"));
    subMenuInsertBefore->setTitle(i18n("Insert Entry Before"));
    menu->addSeparator();
    menu->addMenu(subMenuInsert);
    menu->addMenu(subMenuInsertBefore);
}


#include "worksheetentry.moc"
