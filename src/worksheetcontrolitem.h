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
    Copyright (C) 2020 Sirgienko Nikita <warquark@gmail.com>
 */
#ifndef WORKSHEETCONTROLITEM_H
#define WORKSHEETCONTROLITEM_H

#include <QObject>
#include <QGraphicsRectItem>

class WorksheetEntry;
class Worksheet;

class WorksheetControlItem: public QObject, public QGraphicsRectItem
{
  Q_OBJECT
  public:
    WorksheetControlItem(Worksheet* worksheet, WorksheetEntry* parent);

  Q_SIGNALS:
    void doubleClick();
    void drag(const QPointF, const QPointF);

  private:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

  public:
    bool isSelected{false};
    bool isCollapsable{false};
    bool isCollapsed{false};

  private:
    Worksheet* m_worksheet{nullptr};
    bool m_isHovered{false};
};

#endif // WORKSHEETCONTROLITEM_H
