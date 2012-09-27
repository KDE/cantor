/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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


#ifndef GRAPH2DMOBILE_H
#define GRAPH2DMOBILE_H

#include <QDeclarativeItem>
#include <analitzaplot/plotter2d.h>

class Graph2DMobile : public QDeclarativeItem, public Plotter2D
{
	Q_OBJECT
	Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel);
	Q_PROPERTY(QRectF viewport READ lastViewport WRITE setViewport);
	Q_PROPERTY(bool squares READ squares WRITE setSquares);
	Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio);
	Q_PROPERTY(int currentFunction READ currentFunction WRITE setCurrentFunction)
	public:
		Graph2DMobile(QDeclarativeItem* parent = 0);
		
		virtual void forceRepaint();
		virtual void viewportChanged() {}
		virtual void modelChanged();
		virtual int currentFunction() const { return m_currentFunction; }
		
		virtual void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* w);
		
		void setCurrentFunction(int f) { m_currentFunction = f; }
		
	public slots:
		void translate(qreal x, qreal y);
		void scale(qreal s, int x, int y);
		void resetViewport();
		
	private slots:
		void updateFuncs(const QModelIndex& start, const QModelIndex& end);
		void addFuncs(const QModelIndex& parent, int start, int end);
		void removeFuncs(const QModelIndex& parent, int start, int end);
		
	private:
		bool m_dirty;
		int m_currentFunction;
		
		QPixmap m_buffer;
		QRectF defViewport;
};

#endif // GRAPH2DMOBILE_H
