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

#ifndef GRAPHICS_ITEM_GROUP_OBJECT_H
#define GRAPHICS_ITEM_GROUP_OBJECT_H

/*
 * The purpose of this class is simply to provide a QGraphicsItemGroup
 * that is also an QObject and has an opacity property.
 */

#include <QObject>
#include <QGraphicsItemGroup>

class GraphicsItemGroupObject : public QGraphicsObject
{
    Q_OBJECT
  public:
    GraphicsItemGroupObject(QGraphicsItem* parent);
    ~GraphicsItemGroupObject();

    void setItems(QList<QGraphicsObject*>);

    qreal itemOpacity();
    void setItemOpacity(qreal o);
    QRectF boundingRect() const;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

  private:
    qreal m_itemOpacity;
    Q_PROPERTY(qreal itemOpacity READ itemOpacity WRITE setItemOpacity);
    QList<QGraphicsObject*> m_items;
};


#endif // GRAPHICS_ITEM_GROUP_OBJECT_H
