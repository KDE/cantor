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

  signals:
    void moveToPrevious(int pos, qreal xCoord);
    void moveToNext(int pos, qreal xCoord);
    void receivedFocus(QTextDocument*);
    void tabPressed();
    void execute();
    void sizeChanged();

  protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);

  private:
    void setLocalCursorPosition(const QPointF& pos);
    QPointF localCursorPosition() const;

  private:
    Cantor::Session* session();
};

#endif // WORKSHEET_TEXT_ITEM_H
