/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef WORKSHEETCURSOR_H
#define WORKSHEETCURSOR_H

#include <QTextCursor>

class WorksheetEntry;
class WorksheetTextItem;

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
