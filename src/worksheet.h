/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2017-2021 Alexander Semke <alexander.semke@web.de>
*/

#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QGraphicsScene>
#include <QDomElement>
#include <QGraphicsObject>
#include <QSyntaxHighlighter>
#include <QQueue>

#include "lib/renderer.h"
#include "mathrender.h"
#include "worksheetcursor.h"

namespace Cantor {
    class Backend;
    class Session;
    class Expression;
}

class WorksheetEntry;
class WorksheetView;
class HierarchyEntry;
class PlaceHolderEntry;
class WorksheetTextItem;

class QAction;
class QDrag;
class QMenu;
class QPrinter;
class KActionCollection;
class KToggleAction;
class KFontAction;
class KFontSizeAction;
class KZip;

class Worksheet : public QGraphicsScene
{
  Q_OBJECT
  public:
    enum Type {
      CantorWorksheet,
      JupyterNotebook
    };

    Worksheet(Cantor::Backend*, QWidget*, bool useDeafultWorksheetParameters = true);
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

    void startDrag(WorksheetEntry*, QDrag*);
    void startDragWithHierarchy(HierarchyEntry*, QDrag*, QSizeF responsibleZoneSize);

    void createActions(KActionCollection*);
    QMenu* createContextMenu();
    void populateMenu(QMenu*, QPointF);
    Cantor::Renderer* renderer();
    MathRenderer* mathRenderer();
    bool isEmpty();
    bool isLoadingFromFile();

    WorksheetEntry* currentEntry();
    WorksheetEntry* firstEntry();
    WorksheetEntry* lastEntry();
    WorksheetTextItem* currentTextItem();
    WorksheetTextItem* lastFocusedTextItem();

    WorksheetEntry* cutSubentriesForHierarchy(HierarchyEntry*);
    void insertSubentriesForHierarchy(HierarchyEntry*, WorksheetEntry*);

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
    void setRequestedWidth(QGraphicsObject*, qreal width);
    void removeRequestedWidth(QGraphicsObject*);

    bool isShortcut(const QKeySequence&);

    void setType(Worksheet::Type);
    Worksheet::Type type() const;

    void notifyEntryFocus(WorksheetEntry*);

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

  public:
    static int typeForTagName(const QString&);

  public Q_SLOTS:
    WorksheetEntry* appendCommandEntry();
    void appendCommandEntry(const QString&);
    WorksheetEntry* appendTextEntry();
    WorksheetEntry* appendMarkdownEntry();
    WorksheetEntry* appendImageEntry();
    WorksheetEntry* appendPageBreakEntry();
    WorksheetEntry* appendLatexEntry();
    WorksheetEntry* appendHorizontalRuleEntry();
    WorksheetEntry* appendHierarchyEntry();

    WorksheetEntry* insertCommandEntry(WorksheetEntry* current = nullptr);
    void insertCommandEntry(const QString&);
    WorksheetEntry* insertTextEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertMarkdownEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertImageEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertPageBreakEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertLatexEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertHorizontalRuleEntry(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertHierarchyEntry(WorksheetEntry* current = nullptr);

    WorksheetEntry* insertCommandEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertTextEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertMarkdownEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertImageEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertPageBreakEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertLatexEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertHorizontalRuleEntryBefore(WorksheetEntry* current = nullptr);
    WorksheetEntry* insertHierarchyEntryBefore(WorksheetEntry* current = nullptr);

    void updateLayout();
    void updateHierarchyLayout();
    void updateHierarchyControlsLayout(WorksheetEntry* startEntry = nullptr);
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

    void save(const QString&);
    void save(QIODevice*);
    QByteArray saveToByteArray();
    void savePlain(const QString&);
    void saveLatex(const QString&);
    bool load(QIODevice*);
    void load(QByteArray*);
    bool load(const QString&);

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

    void requestScrollToHierarchyEntry(QString);
    void handleSettingsChanges();

  Q_SIGNALS:
    void modified();
    void loaded();
    void showHelp(const QString&);
    void hierarchyChanged(QStringList, QStringList, QList<int>);
    void hierarhyEntryNameChange(QString name, QString searchName, int depth);
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
    void requestDocumentation(const QString&);

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
    WorksheetEntry* entryAt(QPointF);
    WorksheetEntry* entryAt(int row);
    void updateEntryCursor(QGraphicsSceneMouseEvent*);
    void addEntryFromEntryCursor();
    void drawEntryCursor();
    int entryCount();
    bool loadCantorWorksheet(const KZip& archive);
    bool loadJupyterNotebook(const QJsonDocument& doc);
    void showInvalidNotebookSchemeError(QString additionalInfo = QString());
    void initSession(Cantor::Backend*);
    std::vector<WorksheetEntry*> hierarchySubelements(HierarchyEntry* hierarchyEntry) const;

  private:
    static const double LeftMargin;
    static const double RightMargin;
    static const double TopMargin;
    static const double EntryCursorLength;
    static const double EntryCursorWidth;

    Cantor::Session* m_session{nullptr};
    QSyntaxHighlighter* m_highlighter{nullptr};
    Cantor::Renderer m_epsRenderer;
    MathRenderer m_mathRenderer;
    WorksheetEntry* m_firstEntry{nullptr};
    WorksheetEntry* m_lastEntry{nullptr};
    WorksheetEntry* m_dragEntry{nullptr};
    std::vector<WorksheetEntry*> m_hierarchySubentriesDrag;
    QSizeF m_hierarchyDragSize;
    WorksheetEntry* m_choosenCursorEntry{nullptr};
    bool m_isCursorEntryAfterLastEntry{false};
    QTimer* m_cursorItemTimer;
    QGraphicsLineItem* m_entryCursorItem{nullptr};
    PlaceHolderEntry* m_placeholderEntry{nullptr};
    WorksheetTextItem* m_lastFocusedTextItem{nullptr};
    QTimer* m_dragScrollTimer{nullptr};

    qreal m_viewWidth{0};
    QMap<QGraphicsObject*, qreal> m_itemWidths;
    qreal m_maxWidth{0};
    qreal m_maxPromptWidth{0};

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
    bool m_isClosing{false};
    bool m_readOnly{false};

    Type m_type = CantorWorksheet;

    QString m_backendName;
    QJsonObject* m_jupyterMetadata{nullptr};

    QVector<WorksheetEntry*> m_selectedEntries;
    QQueue<WorksheetEntry*> m_circularFocusBuffer;

    size_t m_hierarchyMaxDepth{0};
};

#endif // WORKSHEET_H
