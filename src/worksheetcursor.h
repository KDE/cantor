/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef WORKSHEETCURSOR_H
#define WORKSHEETCURSOR_H

#include <KTextEditor/Cursor>
#include <KTextEditor/Range>

#include <QTextCursor>

class WorksheetTextEditorItem;
class WorksheetEntry;
class WorksheetTextItem;

class KWorksheetCursor
{
  public:
    KWorksheetCursor();
    KWorksheetCursor(WorksheetEntry*, WorksheetTextEditorItem*, const KTextEditor::Cursor&);
    KWorksheetCursor(WorksheetEntry* entry, WorksheetTextEditorItem* item, const KTextEditor::Cursor& cursor, const KTextEditor::Range& range);
    ~KWorksheetCursor() = default;

    WorksheetEntry* entry() const;
    WorksheetTextEditorItem* textItem() const;
    KTextEditor::Cursor cursor() const;
    KTextEditor::Range foundRange() const;

    bool isValid() const;

  private:
    WorksheetEntry* m_entry;
    WorksheetTextEditorItem* m_textItem;
    KTextEditor::Cursor m_textCursor;
    KTextEditor::Range m_foundRange;
};

class WorksheetCursor
{
  public:
    WorksheetCursor();
    WorksheetCursor(WorksheetEntry*, WorksheetTextItem*, const QTextCursor&);
    ~WorksheetCursor() = default;

    WorksheetEntry* entry() const;
    WorksheetTextItem* textItem() const;
    QTextCursor textCursor() const;

    bool isValid() const;

  private:
    WorksheetEntry* m_entry;
    WorksheetTextItem* m_textItem;
    QTextCursor m_textCursor;
};

#endif // WORKSHEETCURSOR_H
