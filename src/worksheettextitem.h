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

#include "worksheetstatictextitem.h"

namespace Cantor {
    class Session;
}

class WorksheetTextItem : public WorksheetStaticTextItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsLayoutItem)
  public:
    WorksheetTextItem(QGraphicsWidget* parent, QGraphicsLayoutItem* lparent = 0);
    ~WorksheetTextItem();

    void setCursorPosition(const QPointF& pos);
    QPointF cursorPosition() const;

    void setEditable(bool e);
    bool isEditable();

    enum {TopLeft, BottomRight, TopCoord, BottomCoord};

    void setFocusAt(int pos = TopLeft, qreal xCoord = 0);

    void enableCompletion(bool e);
    void activateCompletion(bool a);

    void populateMenu(KMenu *menu);

    QString resolveImages(const QTextCursor& cursor);

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
    void sizeChanged();

  public slots:
    void insertTab();
    void copy();
    void cut();
    void paste();

  protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    bool sceneEvent(QEvent *event);

  private:
    void setLocalCursorPosition(const QPointF& pos);
    QPointF localCursorPosition() const;

    Cantor::Session* session();

  private:
    bool m_completionEnabled;
    bool m_completionActive;
};

#endif // WORKSHEET_TEXT_ITEM_H
