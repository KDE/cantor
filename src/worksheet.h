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

namespace Cantor{
    class Backend;
    class Session;
    class Expression;
}
class WorksheetEntry;
class ResultProxy;

class Worksheet : public KTextEdit
{
  Q_OBJECT
  public:
    Worksheet( Cantor::Backend* backend, QWidget* parent );
    ~Worksheet();

    Cantor::Session* session();

    bool isRunning();
    bool showExpressionIds();

    ResultProxy* resultProxy();

  public slots:
    WorksheetEntry* appendEntry();
    void appendEntry(const QString& text);
    WorksheetEntry* insertEntry();
    void insertEntry(const QString& text);

    void evaluate();
    void evaluateCurrentEntry();
    void evaluateEntry(WorksheetEntry* entry);
    void interrupt();
    void interruptCurrentExpression();

    void enableHighlighting(bool highlight);
    void enableTabCompletion(bool enable);
    void enableExpressionNumbering(bool enable);

    void zoomIn(int range=1);
    void zoomOut(int range=1);

    void save(const QString& filename);
    void savePlain(const QString& filename);
    void load(const QString& filename);

    void gotResult();

  signals:
    void modified();
    void sessionChanged();
    void showHelp(const QString& help);
  
  protected:
    bool event(QEvent* event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

  private slots:
    void removeEntry(QObject* object);
    void checkEntriesForSanity();
  private:
    WorksheetEntry* currentEntry();
    WorksheetEntry* entryAt(const QTextCursor& cursor);
    WorksheetEntry* entryAt(int row);
  private:
    Cantor::Session *m_session;
    ResultProxy* m_proxy;
    QSyntaxHighlighter* m_highlighter;
    QList<WorksheetEntry*> m_entries;
    bool m_tabCompletionEnabled;
    bool m_showExpressionIds;
};

#endif /* _WORKSHEET_H */
