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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */


#ifndef WORKSHEET_TEXT_ITEM_H
#define WORKSHEET_TEXT_ITEM_H

#include <QGraphicsTextItem>

#include <KMenu>

class Worksheet;

namespace Cantor {
    class Session;
}

class WorksheetTextItem : public QGraphicsTextItem
{
  Q_OBJECT
  public:
    WorksheetTextItem(QGraphicsObject* parent,
		      Qt::TextInteractionFlags ti = Qt::NoTextInteraction);
    ~WorksheetTextItem();

    void setCursorPosition(const QPointF& pos);
    QPointF cursorPosition() const;
    QTextCursor cursorForPosition(const QPointF& pos) const;

    enum {TopLeft, BottomRight, TopCoord, BottomCoord};
    enum {Type = UserType + 100};

    int type() const;

    void setFocusAt(int pos = TopLeft, qreal xCoord = 0);

    void enableCompletion(bool b);
    void activateCompletion(bool b);
    void enableDragging(bool b);

    virtual void populateMenu(KMenu *menu, const QPointF& pos);
    QString resolveImages(const QTextCursor& cursor);

    bool isEditable();
    double width();
    double height();

    Worksheet* worksheet();

  signals:
    void moveToPrevious(int pos, qreal xCoord);
    void moveToNext(int pos, qreal xCoord);
    void cursorPositionChanged(QTextCursor);
    void receivedFocus(WorksheetTextItem*);
    void tabPressed();
    void backtabPressed();
    void applyCompletion();
    void doubleClick();
    void execute();
    void appendCommandEntry();
    void deleteEntry();
    void sizeChanged();
    void menuCreated(KMenu*, const QPointF&);
    void drag(const QPointF&, const QPointF&);

  public slots:
    void insertTab();
    void cut();
    void copy();
    void paste();

  protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    //void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
    void dropEvent(QGraphicsSceneDragDropEvent* event);
    bool sceneEvent(QEvent *event);

  private slots:
    void setHeight();
    void testHeight();

  private:
    void setLocalCursorPosition(const QPointF& pos);
    QPointF localCursorPosition() const;

    Cantor::Session* session();

  private:
    bool m_completionEnabled;
    bool m_completionActive;
    bool m_draggingEnabled;
    qreal m_height;
};

#endif // WORKSHEET_TEXT_ITEM_H
