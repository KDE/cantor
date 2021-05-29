/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
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
