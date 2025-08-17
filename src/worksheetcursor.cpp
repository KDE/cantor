/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "worksheetcursor.h"

KWorksheetCursor::KWorksheetCursor()
{
    m_entry = nullptr;
    m_textItem = nullptr;
    m_textCursor = KTextEditor::Cursor::invalid();
    m_foundRange = KTextEditor::Range::invalid();
}

KWorksheetCursor::KWorksheetCursor(WorksheetEntry* entry, WorksheetTextEditorItem* item, const KTextEditor::Cursor& cursor)
{
    m_entry = entry;
    m_textItem = item;
    m_textCursor = cursor;
}

KWorksheetCursor::KWorksheetCursor(WorksheetEntry* entry, WorksheetTextEditorItem* item, const KTextEditor::Cursor& cursor, const KTextEditor::Range& range)
{
    m_entry = entry;
    m_textItem = item;
    m_textCursor = cursor;
    m_foundRange = range;
}

WorksheetEntry* KWorksheetCursor::entry() const
{
    return m_entry;
}

WorksheetTextEditorItem* KWorksheetCursor::textItem() const
{
    return m_textItem;
}

KTextEditor::Cursor KWorksheetCursor::cursor() const
{
    return m_textCursor;
}

KTextEditor::Range KWorksheetCursor::foundRange() const
{
    return m_foundRange;
}

bool KWorksheetCursor::isValid() const
{
    return m_entry && m_textItem && m_textCursor.isValid();
}

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

