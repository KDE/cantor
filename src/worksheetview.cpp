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

#include "worksheetview.h"
#include "worksheet.h"

WorksheetView::WorksheetView(Worksheet* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    m_scale = 1;
    m_worksheet = qobject_cast<Worksheet*>(scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

WorksheetView::~WorksheetView()
{
}

void WorksheetView::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    updateSceneSize();
}

qreal WorksheetView::scaleFactor()
{
    return m_scale;
}

void WorksheetView::updateSceneSize()
{
    QSize s = viewport()->size();
    m_worksheet->setViewSize(s.width() / m_scale, s.height() / m_scale);
}

void WorksheetView::zoomIn()
{
    m_scale *= 1.1;
    scale(1.1, 1.1);
    updateSceneSize();
}

void WorksheetView::zoomOut()
{
    m_scale /= 1.1;
    scale(1/1.1, 1/1.1);
    updateSceneSize();
}

/*
#include "kdebug.h"

void WorksheetView::mousePressEvent(QMouseEvent* event)
{
    QPointF pos = mapToScene(event->pos());
    kDebug() << "Click at" << pos;
    if (QGraphicsItem* item = scene()->itemAt(pos))
	kDebug() << "item " << item;
}
*/

#include "worksheetview.moc"
