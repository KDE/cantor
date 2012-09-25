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

#include "graphicsitemgroupobject.h"

GraphicsItemGroupObject::GraphicsItemGroupObject(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    m_itemOpacity = 1;
}

GraphicsItemGroupObject::~GraphicsItemGroupObject()
{
}

void GraphicsItemGroupObject::setItems(QList<QGraphicsObject*> items)
{
    m_items = items;
}

qreal GraphicsItemGroupObject::itemOpacity()
{
    return m_itemOpacity;
}

void GraphicsItemGroupObject::setItemOpacity(qreal o)
{
    m_itemOpacity = o;
    foreach(QGraphicsObject* item, m_items) {
        item->setOpacity(o);
    }
}

QRectF GraphicsItemGroupObject::boundingRect() const
{
    return QRectF(0,0,0,0);
}

void GraphicsItemGroupObject::paint(QPainter*, const QStyleOptionGraphicsItem*,
                              QWidget*)
{
    return;
}
