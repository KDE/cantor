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
#include <QGraphicsRectItem>

#include <KZip>
#include <QMenu>

#include "worksheetview.h"
#include "epsrenderer.h"
#include "worksheetcursor.h"

namespace Cantor {
    class Backend;
    class Session;
    class Expression;
}

class WorksheetEntry;
class PlaceHolderEntry;
class WorksheetTextItem;

class QAction;
class QDrag;
class QPrinter;
class KActionCollection;
class KToggleAction;
class KFontAction;
class KFontSizeAction;

class Worksheet : public QGraphicsScene
{
  Q_OBJECT
  public:
    Worksheet(Cantor::Backend* backend, QWidget* parent);
    ~Worksheet() override;

    Cantor::Session* session();

    bool isRunning();
    bool showExpressionIds();
    bool animationsEnabled();

    bool isPrinting();

    void setViewSize(qreal w, qreal h, qreal s, bool forceUpdate = false);

    WorksheetView* worksheetView();

    void makeVisible(WorksheetEntry* entry);
    void makeVisible(const WorksheetCursor& cursor);

    void setModified();

    void startDrag(WorksheetEntry* entry, QDrag* drag);

    void createActions(KActionCollection*);
    QMenu* createContextMenu();
    void populateMenu(QMenu* menu, QPointF pos);
    EpsRenderer* epsRenderer();
    bool isEmpty();
    bool isLoadingFromFile();

    WorksheetEntry* currentEntry();
    WorksheetEntry* firstEntry();
    WorksheetEntry* lastEntry();
    WorksheetTextItem* currentTextItem();
    WorksheetTextItem* lastFocusedTextItem();

    WorksheetCursor worksheetCursor();
    void setWorksheetCursor(const WorksheetCursor&);

    void addProtrusion(qreal width);
    void updateProtrusion(qreal oldWidth, qreal newWidth);
    void removeProtrusion(qreal width);

    bool isShortcut(const QKeySequence&);

    // richtext
    struct RichTextInfo {
        bool bold;
        bool italic;
        bool underline;
        bool strikeOut;
        QString font;
        qreal fontSize;
        Qt::Alignment align;
    };

  public Q_SLOTS:
    WorksheetEntry* appendCommandEntry();
    void appendCommandEntry(const QString& text);
    WorksheetEntry* appendTextEntry();
    WorksheetEntry* appendImageEntry();
    WorksheetEntry* appendPageBreakEntry();
    WorksheetEntry* appendLatexEntry();
    WorksheetEntry* insertCommandEntry(WorksheetEntry* current = nullptr);
    void insertCommandEntry(const QString& text);
    WorksheetEntry* insertTextEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertImageEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertPageBreakEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertLatexEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertCommandEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertTextEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertImageEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertPageBreakEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertLatexEntryBefore(WorksheetEntry* current = nullptr);

    void updateLayout();
    void updateEntrySize(WorksheetEntry* entry);

    void print(QPrinter*);

    void focusEntry(WorksheetEntry * entry);

    void evaluate();
    void evaluateCurrentEntry();
    void interrupt();
    void interruptCurrentEntryEvaluation();

    bool completionEnabled();
    //void showCompletion();

    void highlightItem(WorksheetTextItem*);
    void rehighlight();

    void enableHighlighting(bool highlight);
    void enableCompletion(bool enable);
    void enableExpressionNumbering(bool enable);
    void enableAnimations(bool enable);

    QDomDocument toXML(KZip* archive=nullptr);

    void save(const QString& filename);
    void save(QIODevice* device);
    QByteArray saveToByteArray();
    void savePlain(const QString& filename);
    void saveLatex(const QString& filename);
    bool load(QIODevice* device);
    void load(QByteArray* data);
    bool load(const QString& filename);

    void gotResult(Cantor::Expression* expr=nullptr);

    void removeCurrentEntry();

    void setFirstEntry(WorksheetEntry*);
    void setLastEntry(WorksheetEntry*);
    void invalidateFirstEntry();
    void invalidateLastEntry();

    void updateFocusedTextItem(WorksheetTextItem*);

    void updateDragScrollTimer();

    void registerShortcut(QAction*);
    void updateShortcut();

    // richtext
    void setRichTextInformation(const Worksheet::RichTextInfo&);
    void setAcceptRichText(bool b);

    void setTextForegroundColor();
    void setTextBackgroundColor();
    void setTextBold(bool b);
    void setTextItalic(bool b);
    void setTextUnderline(bool b);
    void setTextStrikeOut(bool b);
    void setAlignLeft();
    void setAlignRight();
    void setAlignCenter();
    void setAlignJustify();
    void setFontFamily(const QString&);
    void setFontSize(int size);

  Q_SIGNALS:
    void modified();
    void sessionChanged();
    void showHelp(const QString&);
    void updatePrompt();
    void undoAvailable(bool);
    void redoAvailable(bool);
    void undo();
    void redo();
    void cutAvailable(bool);
    void copyAvailable(bool);
    void pasteAvailable(bool);
    void cut();
    void copy();
    void paste();

  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;

    void dragEnterEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    void dragMoveEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    void dropEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;

    void keyPressEvent(QKeyEvent *keyEvent) Q_DECL_OVERRIDE;

  private Q_SLOTS:
    void loginToSession();
    void showCompletion();
    //void checkEntriesForSanity();

    WorksheetEntry* appendEntry(int type);
    WorksheetEntry* insertEntry(int type, WorksheetEntry* current = nullptr);
    WorksheetEntry* insertEntryBefore(int type, WorksheetEntry* current = nullptr);

  private:
    WorksheetEntry* entryAt(qreal x, qreal y);
    WorksheetEntry* entryAt(QPointF p);
    WorksheetEntry* entryAt(int row);
    void updateEntryCursor(QGraphicsSceneMouseEvent* event);
    void addEntryFromEntryCursor();
    int entryCount();

  private:
    static const double LeftMargin;
    static const double RightMargin;
    static const double TopMargin;
    Cantor::Session *m_session;
    QSyntaxHighlighter* m_highlighter;
    EpsRenderer m_epsRenderer;
    WorksheetEntry* m_firstEntry;
    WorksheetEntry* m_lastEntry;
    WorksheetEntry* m_dragEntry;
    WorksheetEntry* m_choosenCursorEntry;
    QGraphicsLineItem* m_entryCursorItem;
    PlaceHolderEntry* m_placeholderEntry;
    WorksheetTextItem* m_lastFocusedTextItem;
    QTimer* m_dragScrollTimer;

    double m_viewWidth;
    double m_protrusion;
    QMap<qreal, int> m_itemProtrusions;

    QMap<QKeySequence, QAction*> m_shortcuts;

    QList<QAction *> m_richTextActionList;
    KToggleAction* m_boldAction;
    KToggleAction* m_italicAction;
    KToggleAction* m_underlineAction;
    KToggleAction* m_strikeOutAction;
    KFontAction* m_fontAction;
    KFontSizeAction* m_fontSizeAction;
    KToggleAction* m_alignLeftAction;
    KToggleAction* m_alignCenterAction;
    KToggleAction* m_alignRightAction;
    KToggleAction* m_alignJustifyAction;

    bool m_completionEnabled;
    bool m_showExpressionIds;
    bool m_animationsEnabled;
    bool m_loginDone;
    bool m_isPrinting;
    bool m_isLoadingFromFile;
};

#endif // WORKSHEET_H
