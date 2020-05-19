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
#include <QVector>
#include <QQueue>

#include <KZip>
#include <QMenu>

#include "worksheetview.h"
#include "lib/renderer.h"
#include "mathrender.h"
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
    enum Type {
      CantorWorksheet,
      JupyterNotebook
    };

    Worksheet(Cantor::Backend* backend, QWidget* parent, bool useDeafultWorksheetParameters = true);
    ~Worksheet() override;

    Cantor::Session* session();

    void loginToSession();

    bool isRunning();
    bool isReadOnly();
    bool showExpressionIds();
    bool animationsEnabled();
    bool embeddedMathEnabled();

    bool isPrinting();

    WorksheetView* worksheetView();

    void makeVisible(WorksheetEntry*);
    void makeVisible(const WorksheetCursor&);

    void setModified();

    void startDrag(WorksheetEntry* entry, QDrag* drag);

    void createActions(KActionCollection*);
    QMenu* createContextMenu();
    void populateMenu(QMenu* menu, QPointF pos);
    Cantor::Renderer* renderer();
    MathRenderer* mathRenderer();
    bool isEmpty();
    bool isLoadingFromFile();

    WorksheetEntry* currentEntry();
    WorksheetEntry* firstEntry();
    WorksheetEntry* lastEntry();
    WorksheetTextItem* currentTextItem();
    WorksheetTextItem* lastFocusedTextItem();

    WorksheetCursor worksheetCursor();
    void setWorksheetCursor(const WorksheetCursor&);

    // For WorksheetEntry::startDrag
    void resetEntryCursor();

    /**
     * How it works:
     * There are two information streams
     * 1. WorksheetView -> Worksheet -> subelemenets (ex. entries) about view width
     *   View width used by some sub elements for better visual appearance (for example, entries with text often are fitted to width of view).
     * 2. Subelements -> Worksheet
     *   Sub elements notify Worksheet about their needed widths and worksheet, used this information, set proper scene size.
     */
    /// First information stream
    void setViewSize(qreal w, qreal h, qreal s, bool forceUpdate = false);

    /// Second information stream
    void setRequestedWidth(QGraphicsObject* object, qreal width);
    void removeRequestedWidth(QGraphicsObject* object);

    bool isShortcut(const QKeySequence&);

    void setType(Worksheet::Type type);
    Worksheet::Type type() const;

    void notifyEntryFocus(WorksheetEntry* entry);

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
    WorksheetEntry* appendMarkdownEntry();
    WorksheetEntry* appendImageEntry();
    WorksheetEntry* appendPageBreakEntry();
    WorksheetEntry* appendLatexEntry();
    WorksheetEntry* insertCommandEntry(WorksheetEntry* current = nullptr);
    void insertCommandEntry(const QString& text);
    WorksheetEntry* insertTextEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertMarkdownEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertImageEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertPageBreakEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertLatexEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertCommandEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertTextEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertMarkdownEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertImageEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertPageBreakEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertLatexEntryBefore(WorksheetEntry* current = nullptr);

    void updateLayout();
    void updateEntrySize(WorksheetEntry*);

    void print(QPrinter*);
    void paste();
    void focusEntry(WorksheetEntry*);

    void evaluate();
    void evaluateCurrentEntry();
    void interrupt();
    void interruptCurrentEntryEvaluation();

    bool completionEnabled();
    //void showCompletion();

    void highlightItem(WorksheetTextItem*);
    void rehighlight();

    void enableHighlighting(bool);
    void enableCompletion(bool);
    void enableExpressionNumbering(bool);
    void enableAnimations(bool);
    void enableEmbeddedMath(bool);

    QDomDocument toXML(KZip* archive = nullptr);

    void save(const QString& filename);
    void save(QIODevice*);
    QByteArray saveToByteArray();
    void savePlain(const QString& filename);
    void saveLatex(const QString& filename);
    bool load(QIODevice*);
    void load(QByteArray* data);
    bool load(const QString& filename);

    void gotResult(Cantor::Expression* expr = nullptr);

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

    void changeEntryType(WorksheetEntry* target, int newType);

    void collapseSelectionResults();
    void collapseAllResults();
    void uncollapseSelectionResults();
    void uncollapseAllResults();
    void removeSelectionResults();
    void removeAllResults();
    void addToExectuionSelection();
    void excludeFromExecutionSelection();

  Q_SIGNALS:
    void modified();
    void loaded();
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

  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent*) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent*) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent*) override;
    void dropEvent(QGraphicsSceneDragDropEvent*) override;

    void keyPressEvent(QKeyEvent*) override;

    QJsonDocument toJupyterJson();

    bool isValidEntry(WorksheetEntry*);

  private Q_SLOTS:
    void showCompletion();
    //void checkEntriesForSanity();

    WorksheetEntry* appendEntry(int type, bool focus = true);
    WorksheetEntry* insertEntry(int type, WorksheetEntry* current = nullptr);
    WorksheetEntry* insertEntryBefore(int type, WorksheetEntry* current = nullptr);

    //Actions for selection
    void selectionRemove();
    void selectionEvaluate();
    void selectionMoveUp();
    void selectionMoveDown();

    void animateEntryCursor();

  private:
    WorksheetEntry* entryAt(qreal x, qreal y);
    WorksheetEntry* entryAt(QPointF p);
    WorksheetEntry* entryAt(int row);
    void updateEntryCursor(QGraphicsSceneMouseEvent*);
    void addEntryFromEntryCursor();
    void drawEntryCursor();
    int entryCount();
    bool loadCantorWorksheet(const KZip& archive);
    bool loadJupyterNotebook(const QJsonDocument& doc);
    void showInvalidNotebookSchemeError(QString additionalInfo = QString());
    void initSession(Cantor::Backend*);

  private:
    static const double LeftMargin;
    static const double RightMargin;
    static const double TopMargin;
    static const double EntryCursorLength;
    static const double EntryCursorWidth;
    Cantor::Session *m_session;
    QSyntaxHighlighter* m_highlighter;
    Cantor::Renderer m_epsRenderer;
    MathRenderer m_mathRenderer;
    WorksheetEntry* m_firstEntry;
    WorksheetEntry* m_lastEntry;
    WorksheetEntry* m_dragEntry;
    WorksheetEntry* m_choosenCursorEntry;
    bool m_isCursorEntryAfterLastEntry;
    QTimer* m_cursorItemTimer;
    QGraphicsLineItem* m_entryCursorItem;
    PlaceHolderEntry* m_placeholderEntry;
    WorksheetTextItem* m_lastFocusedTextItem;
    QTimer* m_dragScrollTimer;

    qreal m_viewWidth;
    QMap<QGraphicsObject*, qreal> m_itemWidths;
    qreal m_maxWidth;

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

    bool m_useDefaultWorksheetParameters{true};

    bool m_completionEnabled{false};
    bool m_embeddedMathEnabled{false};
    bool m_showExpressionIds{false};
    bool m_animationsEnabled{false};

    bool m_isPrinting{false};
    bool m_isLoadingFromFile{false};
    bool m_readOnly{false};

    Type m_type = CantorWorksheet;

    QString m_backendName;
    QJsonObject* m_jupyterMetadata{nullptr};

    QVector<WorksheetEntry*> m_selectedEntries;
    QQueue<WorksheetEntry*> m_circularFocusBuffer;
};

#endif // WORKSHEET_H
