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

#include "worksheetcontrolitem.h"

#include <KColorScheme>
#include <QApplication>
#include <QDebug>

#include "worksheet.h"
#include "worksheetentry.h"

WorksheetControlItem::WorksheetControlItem(Worksheet* worksheet, WorksheetEntry* parent): QGraphicsRectItem(parent)
{
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    m_worksheet = worksheet;
}

void WorksheetControlItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_worksheet->isPrinting())
        return;

    painter->setViewTransformEnabled(true);

    if (m_isHovered)
        painter->setPen(QPen(QApplication::palette().color(QPalette::Text), 2));
    else
        painter->setPen(QPen(QApplication::palette().color(QPalette::Text), 1));

    qreal x = rect().x();
    qreal y = rect().y();
    qreal w = rect().width();
    qreal h = rect().height();

    painter->drawLine(x, y, x+w, y);
    painter->drawLine(x+w, y, x+w, y+h);
    painter->drawLine(x, y+h, x+w, y+h);

    //For collabsable entries draw "collapsing triangle" (form will depends from collapse's state)
    if (isCollapsable)
    {
        if (isCollapsed)
        {
            QBrush brush = painter->brush();
            brush.setStyle(Qt::SolidPattern);
            brush.setColor(QApplication::palette().color(QPalette::Text));
            painter->setBrush(brush);

            QPolygon triangle;
            triangle << QPoint(x, y) << QPoint(x+w, y) << QPoint(x+w, y+w);

            painter->drawPolygon(triangle);
        }
        else
            painter->drawLine(x, y, x+w, y+w);
    }

    if (isSelected)
    {
        //Use theme colour for selection, but with transparent
        QColor color = QApplication::palette().color(QPalette::Highlight);
        color.setAlpha(192);

        painter->fillRect(rect(), color);
    }
}

void WorksheetControlItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    emit doubleClick();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void WorksheetControlItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->buttons() != Qt::LeftButton)
        return;

    const QPointF buttonDownPos = event->buttonDownPos(Qt::LeftButton);
    if (contains(buttonDownPos) && (event->pos() - buttonDownPos).manhattanLength() >= QApplication::startDragDistance())
    {
        ungrabMouse();
        emit drag(mapToParent(buttonDownPos), mapToParent(event->pos()));
        event->accept();
    }
}

void WorksheetControlItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_isHovered = true;
    update();
}

void WorksheetControlItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_isHovered = false;
    update();
}
