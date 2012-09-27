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

#include "qmlresultitem.h"
#include "commandentry.h"
#include "lib/qmlresult.h"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeItem>

QmlResultItem::QmlResultItem(QGraphicsObject* parent)
    : QGraphicsObject(parent), ResultItem()
{
    m_context = 0;
    m_engine = 0;
    connect(parent, SIGNAL(deleted()), this, SLOT(deleteLater()));
    connect(this, SIGNAL(removeResult()), parentEntry(),
            SLOT(removeResult()));
}

QmlResultItem::~QmlResultItem()
{
}

double QmlResultItem::setGeometry(double x, double y, double w)
{
    setPos(x,y);
    return height();
    // ToDo: Use the width. Notify the worksheet if the qml is
    // too wide.
}

void QmlResultItem::populateMenu(KMenu* menu, const QPointF& pos)
{
    emit menuCreated(menu, mapToParent(pos));
}

ResultItem* QmlResultItem::updateFromResult(Cantor::Result* result)
{
    switch(result->type()) {
    case Cantor::QmlResult::Type:
        {
            Cantor::QmlResult* qmlresult;
            qmlresult = dynamic_cast<Cantor::QmlResult*>(result);
            m_engine = new QDeclarativeEngine(this);
            m_context = new QDeclarativeContext(m_engine, this);
            foreach(Cantor::ContextProperty prop, qmlresult->properties()) {
                m_context->setContextProperty(prop.name, prop.value);
            }
            QDeclarativeComponent component(m_engine);
            component.setData(qmlresult->qml().toUtf8(), QString());
            QObject* item = component.create(m_context);
            m_qmlItem = qobject_cast<QDeclarativeItem*>(item);
            m_qmlItem->setParent(this);
            return this;
        }
    default:
        deleteLater();
        return create(parentEntry(), result);
    }
}

double QmlResultItem::width() const
{
    if (m_qmlItem)
        return m_qmlItem->property("width").toReal();
    else
        return 0;
}

double QmlResultItem::height() const
{
    if (m_qmlItem)
        return m_qmlItem->property("height").toReal();
    else
        return 0;
}

QRectF QmlResultItem::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}

void QmlResultItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o,
                          QWidget* w)
{
    if (m_qmlItem)
        m_qmlItem->paint(p, o, w);
}

void QmlResultItem::deleteLater()
{
    QGraphicsObject::deleteLater();
}

CommandEntry* QmlResultItem::parentEntry()
{
    return qobject_cast<CommandEntry*>(parentObject());
}

Cantor::Result* QmlResultItem::result()
{
    return parentEntry()->expression()->result();
}

#include "qmlresultitem.moc"
