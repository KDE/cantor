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

#include <ktextedit.h>
#include <QHash>

namespace MathematiK{
    class Backend;
    class Session;
    class Expression;
}
class WorksheetEntry;

class Worksheet : public KTextEdit
{
  Q_OBJECT
  public:
    Worksheet( MathematiK::Backend* backend, QWidget* parent );
    ~Worksheet();

    MathematiK::Session* session();

    bool isRunning();
    bool showExpressionIds();

  public slots:
    void appendEntry(const QString& text=QString());
    void insertEntry(const QString& text=QString());

    void evaluate();
    void evaluateCurrentEntry();
    void interrupt();
    void interruptCurrentExpression();

    void enableHighlighting(bool highlight);
    void enableTabCompletion(bool enable);
    void enableExpressionNumbering(bool enable);

    void save(const QString& filename);
    void load(const QString& filename);

    void gotResult();

  signals:
    void modified();
    void sessionChanged();
    void showHelp(const QString& help);
  
  protected:
    bool event(QEvent* event);
    void keyPressEvent(QKeyEvent *event);

  private slots:
    void removeEntry(QObject* object);
    void checkEntriesForSanity();
  private:
    WorksheetEntry* currentEntry();
    WorksheetEntry* entryAt(int row);
  private:
    MathematiK::Session *m_session;
    QSyntaxHighlighter* m_highlighter;
    QList<WorksheetEntry*> m_entries;
    bool m_tabCompletionEnabled;
    bool m_showExpressionIds;
};

#endif /* _WORKSHEET_H */
