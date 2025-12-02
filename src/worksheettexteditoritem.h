#ifndef WORKSHEET_TEXT_EDITOR_ITEM_H
#define WORKSHEET_TEXT_EDITOR_ITEM_H

#include <KTextEditor/Attribute>
#include <KTextEditor/Cursor>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/Range>
#include <KTextEditor/View>
#include <KTextEditor/MovingRange>
#include "cantorcompletionmodel.h"
#include <KSyntaxHighlighting/Theme>

#include <QGraphicsProxyWidget>
#include <QMenu>
#include <QListWidget>
#include <KStandardAction>
#include <QFont>
#include <QVector>

class Worksheet;
class WorksheetEntry;
class WorksheetView;
class KWorksheetCursor;
class DocumentWordCompletionModel;

namespace Cantor {
    class Session;
}

class WorksheetTextEditorItem : public QGraphicsProxyWidget
{
    Q_OBJECT
public:
    enum DoubleClickEventBehaviour {
        Simple,
        ImageReplacement
    };
    Q_ENUM(DoubleClickEventBehaviour)

    enum EditorMode {
        ReadOnly,
        Editable
    };
    Q_ENUM(EditorMode)

    enum {
        TopLeft,
        BottomRight,
        TopCoord,
        BottomCoord
    };

    explicit WorksheetTextEditorItem(EditorMode initialMode, WorksheetEntry* parentEntry, QGraphicsItem* parent = nullptr);
    ~WorksheetTextEditorItem() override;

    KTextEditor::View* view() const;
    KTextEditor::Document* document() const;

    QString toPlainText()   const;
    void setPlainText(const QString&);
    void clear();
    void insertText(const QString&);
    void clearSelection();

    void setBackgroundColor(const QColor&);
    QColor backgroundColor() const;
    QColor themeBackgroundColor() const { return m_themeDefaultBackgroundColor; }
    void setDefaultTextColor(const QColor&);
    QColor defaultTextColor() const;

    QPointF localCursorPosition() const;
    QRectF cursorRect() const;
    QRectF sceneCursorRect() const;
    void setFocusAt(int pos = TopLeft, qreal xCoord = 0);

    void setDoubleClickBehaviour(DoubleClickEventBehaviour behaviour);

    EditorMode currentEditorMode() const;
    bool isEditable() const;
    bool isReadOnly() const;
    bool completionEnabled() const;
    bool completionActive() const;
    bool isDragEnabled() const;

    void setReadOnly(bool readOnly);
    void enableCompletion(bool enable);
    void activateCompletion(bool activate);
    void setDragEnabled(bool enable);
    KTextEditor::CodeCompletionModel* codeCompletionModel() const;
    void setSyntaxHighlightingMode(const QString& modeName);
    QString syntaxHighlightingMode() const;

    virtual void populateMenu(QMenu* , QPointF);

    double width() const;
    double height() const;
    QSizeF estimateContentSize(qreal maxWidth) const;
    qreal setGeometry(qreal x, qreal y, qreal w, bool centered = false);

    Worksheet* worksheet();
    WorksheetView* worksheetView();
    WorksheetEntry* worksheetEntry();

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    bool isCutAvailable() const;
    bool isCopyAvailable() const;
    bool isPasteAvailable() const;

    KTextEditor::Range search(const QString& pattern, KTextEditor::SearchOptions options, const KTextEditor::Cursor& start = KTextEditor::Cursor());
    bool replace(const QString& replacement);
    void replaceAll(const QString& pattern, const QString& replacement, KTextEditor::SearchOptions options);


    void setTextForegroundColor();
    void setTextBackgroundColor();
    void setTextBold(bool bold);
    void setTextItalic(bool italic);
    void setTextUnderline(bool underline);
    void setTextStrikeOut(bool strikeOut);
    void setAlignment(Qt::Alignment alignment);
    void setFontFamily(const QString& family);
    void setFontSize(int size);

    QColor themeDefaultTextColor() const { return m_themeDefaultTextColor; }
    void setFont(const QFont& font);
    void setTheme(const QString& themeName);
    void increaseFontSize();
    void decreaseFontSize();
Q_SIGNALS:
    void moveToPrevious(int pos, qreal xCoord);
    void moveToNext(int pos, qreal xCoord);
    void cursorPositionChanged(const KTextEditor::Cursor& pos);
    void receivedFocus(WorksheetTextEditorItem* item);
    void tabPressed();
    void backtabPressed();
    void applyCompletion();
    void doubleClick();
    void execute();
    void deleteEntry();
    void sizeChanged();
    void menuCreated(QMenu*, QPointF);
    void drag(const QPointF& startPos, const QPointF& currentPos);

    void undoAvailable(bool);
    void redoAvailable(bool);
    void cutAvailable(bool);
    void copyAvailable(bool);
    void pasteAvailable(bool);

    void textContentModified();
    void modificationStatysChanged(bool isModified);
    void editorSelectionChanged();

public Q_SLOTS:
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectionChanged();
    void testSize();
    void clipboardChanged();

private Q_SLOTS:
    void showCustomCompleter(const QList<CantorCompletionModel::CompletionItem>&);
    void onCompleterItemSelected();
    void hideCompleterAndResetFocus();
private:
    void setupLineHeight();
    void applyFontState();
    void resetScrollPosition();
protected:
    bool eventFilter(QObject*, QEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent*) override;
    void dropEvent(QGraphicsSceneDragDropEvent*) override;
    bool sceneEvent(QEvent*) override;
    void wheelEvent(QGraphicsSceneWheelEvent*) override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
    QPainterPath shape() const override;
private:
    QKeyEvent* eventForStandardAction(KStandardAction::StandardAction);
    Cantor::Session* session();
    QListWidget* m_customCompleter{nullptr};

    KTextEditor::Document* m_document{nullptr};
    KTextEditor::View* m_view{nullptr};
    KTextEditor::Editor* m_editor{nullptr};
    CantorCompletionModel* m_completionModel{nullptr};

    WorksheetEntry* m_parentEntry{nullptr};
    EditorMode m_mode ;
    DoubleClickEventBehaviour m_dblClickBehaviour = ImageReplacement;
    bool m_completionEnabled = true;
    bool m_completionActive = false;
    bool m_dragEnabled = false;
    QColor m_defaultTextColor;
    QColor m_themeDefaultTextColor;
    QSizeF m_size;
    QColor m_themeDefaultBackgroundColor;

    int m_currentFontPointSize = 10;
    QFont m_currentFont;
    KTextEditor::MovingRange* m_defaultTextColorRange{nullptr};
    KTextEditor::Attribute::Ptr m_defaultTextColorAttribute;
};

#endif // WORKSHEET_TEXT_EDITOR_ITEM_H
