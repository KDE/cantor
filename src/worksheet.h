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
 */

#ifndef _WORKSHEET_H
#define _WORKSHEET_H

#include <QTextEdit>
#include <QHash>

namespace MathematiK{
    class Backend;
    class Session;
    class Expression;
}
class WorksheetEntry;

class Worksheet : public QTextEdit
{
  Q_OBJECT
  public:
    Worksheet( MathematiK::Backend* backend, QWidget* parent );
    ~Worksheet();

    bool event(QEvent* event);

    void appendEntry(const QString& text=QString());

    MathematiK::Session* session();

    bool isRunning();

    QTextTable* mainTable();

  public slots:
    void evaluate();
    void interrupt();
    void interruptCurrentExpression();

    void save(const QString& filename);
    void load(const QString& filename);

    void gotResult();

  signals:
    void modified();
    void sessionChanged();
    void showHelp(const QString& help);
  private slots:
    void removeEntry(QObject* object);
  private:
    void initMainTable();
    void evaluateCurrentEntry();
    WorksheetEntry* currentEntry();
    WorksheetEntry* entryAt(int row);
  private:
    MathematiK::Session *m_session;
    QTextTable* m_mainTable;
    QList<WorksheetEntry*> m_entries;
    
};

#endif /* _WORKSHEET_H */
