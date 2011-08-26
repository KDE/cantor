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

#include <krichtextwidget.h>
#include <QHash>

namespace Cantor{
    class Backend;
    class Session;
    class Expression;
}
class WorksheetEntry;
class ResultProxy;
class TextEntry;

class Worksheet : public KRichTextWidget
{
  Q_OBJECT
  public:
    Worksheet( Cantor::Backend* backend, QWidget* parent );
    ~Worksheet();

    Cantor::Session* session();

    bool isRunning();
    bool showExpressionIds();

    ResultProxy* resultProxy();

    void print(QPrinter* printer);

    bool isPrinting();

  public slots:
    WorksheetEntry* appendCommandEntry();
    void appendCommandEntry(const QString& text);
    WorksheetEntry* appendTextEntry();
    WorksheetEntry* appendImageEntry();
    WorksheetEntry* appendPageBreakEntry();
    WorksheetEntry* appendLatexEntry();
    WorksheetEntry* insertCommandEntry();
    void insertCommandEntry(const QString& text);
    WorksheetEntry* insertTextEntry();
    WorksheetEntry* insertImageEntry();
    WorksheetEntry* insertPageBreakEntry();
    WorksheetEntry* insertLatexEntry();
    WorksheetEntry* insertCommandEntryBefore();
    WorksheetEntry* insertTextEntryBefore();
    WorksheetEntry* insertImageEntryBefore();
    WorksheetEntry* insertPageBreakEntryBefore();
    WorksheetEntry* insertLatexEntryBefore();

    void setCurrentEntry(WorksheetEntry * entry, bool moveCursor = true);
    void moveToPreviousEntry();
    void moveToNextEntry();

    void evaluate();
    void evaluateCurrentEntry();
    void interrupt();
    void interruptCurrentEntryEvaluation();

    bool completionEnabled();
    void showCompletion();

    void enableHighlighting(bool highlight);
    void enableCompletion(bool enable);
    void enableExpressionNumbering(bool enable);

    void zoomIn(int range=1);
    void zoomOut(int range=1);

    QDomDocument toXML(KZip* archive=0);

    void save(const QString& filename);
    void savePlain(const QString& filename);
    void saveLatex(const QString& filename, bool exportImages);
    void load(const QString& filename);

    void gotResult(Cantor::Expression* expr=0);

    void removeCurrentEntry();

  signals:
    void modified();
    void sessionChanged();
    void showHelp(const QString& help);
    void updatePrompt();

  protected:
    bool event(QEvent* event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent *event);

  private slots:
    void loginToSession();
    void removeEntry(QObject* object);
    void checkEntriesForSanity();

    WorksheetEntry* insertEntryAt(int type, const QTextCursor& cursor);
    WorksheetEntry* appendEntry(int type);
    WorksheetEntry* insertEntry(int type);
    WorksheetEntry* insertEntryBefore(int type);
  private:
    WorksheetEntry* currentEntry();
    WorksheetEntry* entryAt(const QTextCursor& cursor);
    WorksheetEntry* entryAt(int row);
    WorksheetEntry* entryNextTo(const QTextCursor& cursor);
  private:
    Cantor::Session *m_session;
    ResultProxy* m_proxy;
    QSyntaxHighlighter* m_highlighter;
    QList<WorksheetEntry*> m_entries;
    WorksheetEntry* m_currentEntry;
    bool m_completionEnabled;
    bool m_showExpressionIds;
    bool m_loginFlag;
    bool m_isPrinting;
};

#endif /* _WORKSHEET_H */
