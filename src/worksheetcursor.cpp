/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "worksheetcursor.h"

WorksheetCursor::WorksheetCursor()
{
    m_entry = nullptr;
    m_textItem = nullptr;
    m_textCursor = QTextCursor();
}

WorksheetCursor::WorksheetCursor(WorksheetEntry* entry, WorksheetTextItem* item,
                                 const QTextCursor& cursor)
{
    m_entry = entry;
    m_textItem = item;
    m_textCursor = cursor;
}

WorksheetEntry* WorksheetCursor::entry() const
{
    return m_entry;
}

WorksheetTextItem* WorksheetCursor::textItem() const
{
    return m_textItem;
}

QTextCursor WorksheetCursor::textCursor() const
{
    return m_textCursor;
}

bool WorksheetCursor::isValid() const
{
    return m_entry && m_textItem;
}

