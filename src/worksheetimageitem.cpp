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

#include "worksheetimageitem.h"
#include "worksheet.h"

#include <QMovie>
#include <QImage>
#include <QGraphicsSceneContextMenuEvent>
#include <KUrl>
#include <KMenu>
#include <KDebug>

WorksheetImageItem::WorksheetImageItem(QGraphicsObject* parent)
    : QGraphicsObject(parent)
{
    connect(this, SIGNAL(menuCreated(KMenu*, const QPointF&)), parent,
	    SLOT(populateMenu(KMenu*, const QPointF&)), Qt::DirectConnection);
}

WorksheetImageItem::~WorksheetImageItem()
{
}

int WorksheetImageItem::type() const
{
    return Type;
}

qreal WorksheetImageItem::height() const
{
    return m_image.height();
}

qreal WorksheetImageItem::width() const
{
    return m_image.width();
}

void WorksheetImageItem::paint(QPainter *painter, 
			       const QStyleOptionGraphicsItem *option, 
			       QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawImage(QRectF(QPointF(0,0), m_size), m_image,
			m_image.rect());
}

void WorksheetImageItem::setEps(const KUrl& url)
{
    const QImage img = worksheet()->epsRenderer()->renderToImage(url, &m_size);
    m_image = img;
}

void WorksheetImageItem::setImage(QImage img)
{
    m_image = img;
    m_size = m_image.size();
}

void WorksheetImageItem::setPixmap(QPixmap pixmap)
{
    m_image = pixmap.toImage();
    m_size = m_image.size();
}

void WorksheetImageItem::populateMenu(KMenu *menu, const QPointF& pos)
{
    emit menuCreated(menu, mapToParent(pos));
}

void WorksheetImageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    KMenu *menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());

    menu->popup(event->screenPos());
}

Worksheet* WorksheetImageItem::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}

#include "worksheetimageitem.moc"

