/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/


#ifndef WORKSHEET_TEXT_ITEM_H
#define WORKSHEET_TEXT_ITEM_H

#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QTextCursor>

#include <QMenu>
#include <KStandardAction>

class Worksheet;
class WorksheetEntry;
class WorksheetView;
class WorksheetCursor;

namespace Cantor {
    class Session;
}

class QTextCharFormat;

class WorksheetTextItem : public QGraphicsTextItem
{
  Q_OBJECT
  public:
    enum DoubleClickEventBehaviour {Simple, ImageReplacement};

    explicit WorksheetTextItem(WorksheetEntry* parent,
                      Qt::TextInteractionFlags ti = Qt::NoTextInteraction);
    ~WorksheetTextItem() override;

    void setCursorPosition(QPointF);
    QPointF cursorPosition() const;
    QTextCursor cursorForPosition(QPointF) const;
    QRectF sceneCursorRect(QTextCursor cursor = QTextCursor()) const;
    QRectF cursorRect(QTextCursor cursor = QTextCursor()) const;

    enum {TopLeft, BottomRight, TopCoord, BottomCoord};
    enum {Type = UserType + 100};

    int type() const override;

    void setFocusAt(int pos = TopLeft, qreal xCoord = 0);

    void enableCompletion(bool b);
    void activateCompletion(bool b);
    void setItemDragable(bool b);
    void enableRichText(bool b);

    virtual void populateMenu(QMenu*, QPointF);
    QString resolveImages(const QTextCursor&);

    bool isEditable();
    void allowEditing();
    void denyEditing();
    void setBackgroundColor(const QColor&);
    const QColor& backgroundColor() const;
    bool richTextEnabled();
    double width() const;
    double height() const;
    virtual qreal setGeometry(qreal x, qreal y, qreal w, bool centered=false);

    Worksheet* worksheet();
    WorksheetView* worksheetView();

    void clearSelection();

    bool isUndoAvailable();
    bool isRedoAvailable();
    bool isCutAvailable();
    bool isCopyAvailable();
    bool isPasteAvailable();

    // richtext
    void setTextForegroundColor();
    void setTextBackgroundColor();
    void setTextBold(bool);
    void setTextItalic(bool);
    void setTextUnderline(bool);
    void setTextStrikeOut(bool);
    void setAlignment(Qt::Alignment);
    void setFontFamily(const QString&);
    void setFontSize(int);

    QTextCursor search(QString pattern,
                       QTextDocument::FindFlags qt_flags,
                       const WorksheetCursor& pos);

    DoubleClickEventBehaviour doubleClickBehaviour();
    void setDoubleClickBehaviour(DoubleClickEventBehaviour);

  Q_SIGNALS:
    void moveToPrevious(int pos, qreal xCoord);
    void moveToNext(int pos, qreal xCoord);
    void cursorPositionChanged(QTextCursor);
    void receivedFocus(WorksheetTextItem*);
    void tabPressed();
    void backtabPressed();
    void applyCompletion();
    void doubleClick();
    void execute();
    void deleteEntry();
    void sizeChanged();
    void menuCreated(QMenu*, QPointF);
    void drag(const QPointF&, QPointF);
    void undoAvailable(bool);
    void redoAvailable(bool);
    void cutAvailable(bool);
    void copyAvailable(bool);
    void pasteAvailable(bool);

  public Q_SLOTS:
    void insertTab();
    void cut();
    void copy();
    void paste();
    void undo();
    void redo();
    void clipboardChanged();
    void selectionChanged();
    void testSize();

  protected:
    void keyPressEvent(QKeyEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent*) override;
    //void dragLeaveEvent(QGraphicsSceneDragDropEvent*);
    void dragMoveEvent(QGraphicsSceneDragDropEvent*) override;
    void dropEvent(QGraphicsSceneDragDropEvent*) override;
    bool sceneEvent(QEvent*) override;
    void wheelEvent(QGraphicsSceneWheelEvent*) override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

  private Q_SLOTS:
    //void setHeight();
    void updateRichTextActions(QTextCursor cursor);

  private:
    void setLocalCursorPosition(QPointF);
    QPointF localCursorPosition() const;

    QKeyEvent* eventForStandardAction(KStandardAction::StandardAction);
    Cantor::Session* session();

    // richtext
    void mergeFormatOnWordOrSelection(const QTextCharFormat&);

  private:
    QSizeF m_size;
    bool m_completionEnabled{false};
    bool m_completionActive{false};
    bool m_itemDragable{false};
    bool m_richTextEnabled{false};
    QColor m_backgroundColor;
    DoubleClickEventBehaviour m_eventBehaviour{DoubleClickEventBehaviour::ImageReplacement};
};

#endif // WORKSHEET_TEXT_ITEM_H
