/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "worksheettoolbutton.h"
#include <QPixmap>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

WorksheetToolButton::WorksheetToolButton(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    m_size = QSize(16, 16);
    setCursor(QCursor(Qt::ArrowCursor));
    m_scale = 0;
}

void WorksheetToolButton::setIcon(const QIcon& icon)
{
    m_icon = icon;
}

qreal WorksheetToolButton::width()
{
    return m_size.width();
}

qreal WorksheetToolButton::height()
{
    return m_size.height();
}

QRectF WorksheetToolButton::boundingRect() const
{
    return QRectF(0, 0, m_size.width(), m_size.height());
}

void WorksheetToolButton::setIconScale(qreal scale)
{
    m_scale = scale;
    m_pixmap = m_icon.pixmap(m_size * m_scale);
}

void WorksheetToolButton::paint(QPainter* painter,
                                const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_scale == 0)
        setIconScale(1);
    QRectF rect(QPointF(0,0), m_size);
    painter->drawPixmap(rect, m_pixmap, m_pixmap.rect());
}

void WorksheetToolButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    emit pressed();
}

void WorksheetToolButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (boundingRect().contains(event->pos()))
        emit clicked();
}
