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

#ifndef QMLRESULTITEM_H
#define QMLRESULTITEM_H

#include "resultitem.h"

#include <QDeclarativeItem>

class QDeclarativeContext;
class QDeclarativeEngine;
class QMenu;

class CommandEntry;

namespace Cantor {
    class QmlResult;
}

class QmlResultItem : public QGraphicsObject, public ResultItem
{
  Q_OBJECT
  public:
    QmlResultItem(QGraphicsObject* parent);
    ~QmlResultItem();
    double setGeometry(double x, double y, double w);
    void populateMenu(KMenu* menu, const QPointF& pos);

    ResultItem* updateFromResult(Cantor::Result* result);

    double width() const;
    double height() const;

    QRectF boundingRect() const;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

    void deleteLater();
    CommandEntry* parentEntry();
    Cantor::Result* result();
  signals:
    void removeResult();
    void menuCreated(QMenu*, QPointF);
  private:
    QDeclarativeContext* m_context;
    QDeclarativeEngine* m_engine;
    QDeclarativeItem* m_qmlItem;
};

#endif //QMLRESULTITEM_H
