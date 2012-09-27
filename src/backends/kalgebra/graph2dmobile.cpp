/*************************************************************************************
 *  Copyright (C) 2011 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "graph2dmobile.h"
#include <QPainter>
#include <qstyleoption.h>
#include <QEvent>

Graph2DMobile::Graph2DMobile(QDeclarativeItem* parent)
	: QDeclarativeItem(parent), Plotter2D(boundingRect().size())
	, m_dirty(true), m_currentFunction(-1)
{
	setSize(QSizeF(100,100));
	setFlag(QGraphicsItem::ItemHasNoContents, false);
	
	defViewport = QRectF(QPointF(-12., 10.), QSizeF(24., -20.));
	resetViewport();
}

void Graph2DMobile::paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* )
{
// 	qDebug() << "hellooo" << boundingRect();
	if(boundingRect().size().isEmpty())
		return;
	
	if(m_buffer.size()!=boundingRect().size()) {
		m_buffer = QPixmap(boundingRect().size().toSize());
		setPaintedSize(boundingRect().size().toSize());
	}
	
	Q_ASSERT(!m_buffer.isNull());
	
	if(m_dirty) {
		m_buffer.fill(Qt::transparent);
		drawFunctions(&m_buffer);
		m_dirty=false;
	}
	
	p->drawPixmap(QPoint(0,0), m_buffer);
}

void Graph2DMobile::forceRepaint()
{
	m_dirty=true;
	update();
}

void Graph2DMobile::resetViewport()
{
	setViewport(defViewport);
}

void Graph2DMobile::modelChanged()
{
	connect(model(), SIGNAL(dataChanged( const QModelIndex&, const QModelIndex& )),
		this, SLOT(updateFuncs(const QModelIndex&, const QModelIndex)));
	connect(model(), SIGNAL( rowsInserted ( const QModelIndex &, int, int ) ),
		this, SLOT(addFuncs(const QModelIndex&, int, int)));
	connect(model(), SIGNAL( rowsRemoved ( const QModelIndex &, int, int ) ),
		this, SLOT(removeFuncs(const QModelIndex&, int, int)));
}

void Graph2DMobile::addFuncs(const QModelIndex& parent, int start, int end) { updateFunctions(parent, start, end); }

void Graph2DMobile::removeFuncs(const QModelIndex&, int, int) { forceRepaint(); }
void Graph2DMobile::updateFuncs(const QModelIndex& start, const QModelIndex& end) { updateFunctions(QModelIndex(), start.row(), end.row()); }

void Graph2DMobile::scale(qreal s, int x, int y)
{
	QRectF userViewport = lastUserViewport();
	if(s<1 || (userViewport.height() < -3. && userViewport.width() > 3.)) {
		scaleViewport(s, QPoint(x,y));
	}
}

void Graph2DMobile::translate(qreal x, qreal y)
{
	moveViewport(QPoint(x,y));
}
