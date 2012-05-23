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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QGraphicsScene>
#include <QDomElement>
#include <QGraphicsLinearLayout>
#include <QSyntaxHighlighter>

#include <KZip>
#include <KMenu>

#include "worksheetview.h"
#include "epsrenderer.h"

namespace Cantor {
    class Backend;
    class Session;
    class Expression;
}

class ResultProxy;
class WorksheetEntry;
class WorksheetTextItem;

class Worksheet : public QGraphicsScene
{
  Q_OBJECT
  public:
    Worksheet(Cantor::Backend* backend, QWidget* parent);
    ~Worksheet();

    Cantor::Session* session();

    ResultProxy* resultProxy();

    bool isRunning();
    bool showExpressionIds();

    void print(QPrinter* printer);

    bool isPrinting();

    void setViewSize(qreal w, qreal h);

    WorksheetView* worksheetView();

    void setModified();

    KMenu* createContextMenu();
    void populateMenu(KMenu* menu);
    EpsRenderer* epsRenderer();
    qreal contentsWidth();

  public slots:
    WorksheetEntry* appendCommandEntry();
    void appendCommandEntry(const QString& text);
    WorksheetEntry* appendTextEntry();
    //WorksheetEntry* appendImageEntry();
    //WorksheetEntry* appendPageBreakEntry();
    WorksheetEntry* appendLatexEntry();
    WorksheetEntry* insertCommandEntry();
    void insertCommandEntry(const QString& text);
    WorksheetEntry* insertTextEntry();
    //WorksheetEntry* insertImageEntry();
    //WorksheetEntry* insertPageBreakEntry();
    WorksheetEntry* insertLatexEntry();
    WorksheetEntry* insertCommandEntryBefore();
    WorksheetEntry* insertTextEntryBefore();
    //WorksheetEntry* insertImageEntryBefore();
    //WorksheetEntry* insertPageBreakEntryBefore();
    WorksheetEntry* insertLatexEntryBefore();

    void focusEntry(WorksheetEntry * entry);

    void evaluate();
    void evaluateCurrentEntry();
    void interrupt();
    void interruptCurrentEntryEvaluation();

    bool completionEnabled();
    //void showCompletion();

    void highlightItem(WorksheetTextItem*);

    void enableHighlighting(bool highlight);
    void enableCompletion(bool enable);
    void enableExpressionNumbering(bool enable);

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
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

  private slots:
    void loginToSession();
    void showCompletion();
    //void checkEntriesForSanity();

    WorksheetEntry* appendEntry(int type);
    WorksheetEntry* insertEntry(int type);
    WorksheetEntry* insertEntryBefore(int type);

  private:
    WorksheetEntry* currentEntry();
    WorksheetEntry* firstEntry();
    WorksheetEntry* lastEntry();
    WorksheetEntry* entryAt(qreal x, qreal y);
    WorksheetEntry* entryAt(int row);
    int entryCount();

  private:
    Cantor::Session *m_session;
    ResultProxy* m_proxy;
    QGraphicsWidget* m_rootwidget;
    QGraphicsLinearLayout* m_rootlayout;
    QSyntaxHighlighter* m_highlighter;
    EpsRenderer m_epsRenderer;

    bool m_completionEnabled;
    bool m_showExpressionIds;
    bool m_loginFlag;
    bool m_isPrinting;
};

#endif // WORKSHEET_H
