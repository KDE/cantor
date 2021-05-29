/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2021 Alexander Semke <alexander.semke@web.de>
*/

#include "worksheetview.h"
#include "worksheet.h"

#include <QFocusEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollBar>

WorksheetView::WorksheetView(Worksheet* scene, QWidget* parent) : QGraphicsView(scene, parent),
    m_worksheet(scene)
{
    connect(scene, SIGNAL(sceneRectChanged(QRectF)),
            this, SLOT(sceneRectChanged(QRectF)));
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void WorksheetView::makeVisible(const QRectF& sceneRect)
{
    const qreal w = viewport()->width();
    const qreal h = viewport()->height();

    QRectF rect(m_scale*sceneRect.topLeft(), m_scale*sceneRect.size());

    qreal x,y;
    if (m_animation) {
        x = m_hAnimation->endValue().toReal();
        y = m_vAnimation->endValue().toReal();

        if (QRectF(x,y,w,h).contains(rect))
            return;
    }

    if (horizontalScrollBar())
        x = horizontalScrollBar()->value();
    else
        x = 0;
    if (verticalScrollBar())
        y = verticalScrollBar()->value();
    else
        y = 0;

    if (!m_animation && QRectF(x,y,w,h).contains(rect))
        return;

    qreal nx, ny;
    if (y > rect.y() || rect.height() > h)
        ny = rect.y();
    else
        ny = rect.y() + rect.height() - h;
    if (rect.x() + rect.width() <= w || x > rect.x())
        nx = 0;
    else
        nx = rect.x() + rect.width() - w;

    if (!m_worksheet->animationsEnabled()) {
        if (horizontalScrollBar())
            horizontalScrollBar()->setValue(nx);
        if (verticalScrollBar())
            verticalScrollBar()->setValue(ny);
        return;
    }

    if (!m_animation)
        m_animation = new QParallelAnimationGroup(this);

    if (horizontalScrollBar()) {
        if (!m_hAnimation) {
            m_hAnimation = new QPropertyAnimation(horizontalScrollBar(),
                                                  "value", this);
            m_hAnimation->setStartValue(horizontalScrollBar()->value());
            nx = qBound(qreal(0.0), nx, qreal(0.0+horizontalScrollBar()->maximum()));
            m_hAnimation->setEndValue(nx);
            m_hAnimation->setDuration(100);
            m_animation->addAnimation(m_hAnimation);
        } else {
            qreal progress = static_cast<qreal>(m_hAnimation->currentTime()) /
                m_hAnimation->totalDuration();
            QEasingCurve curve = m_hAnimation->easingCurve();
            qreal value = curve.valueForProgress(progress);
            qreal sx = 1/(1-value)*(m_hAnimation->currentValue().toReal() -
                                    value * nx);
            m_hAnimation->setStartValue(sx);
            m_hAnimation->setEndValue(nx);
        }
    } else {
        m_hAnimation = nullptr;
    }

    if (verticalScrollBar()) {
        if (!m_vAnimation) {
            m_vAnimation = new QPropertyAnimation(verticalScrollBar(),
                                                  "value", this);
            m_vAnimation->setStartValue(verticalScrollBar()->value());
            ny = qBound(qreal(0.0), ny, qreal(0.0+verticalScrollBar()->maximum()));
            m_vAnimation->setEndValue(ny);
            m_vAnimation->setDuration(100);
            m_animation->addAnimation(m_vAnimation);
        } else {
            qreal progress = static_cast<qreal>(m_vAnimation->currentTime()) /
                m_vAnimation->totalDuration();
            QEasingCurve curve = m_vAnimation->easingCurve();
            qreal value = curve.valueForProgress(progress);
            qreal sy = 1/(1-value)*(m_vAnimation->currentValue().toReal() -
                                    value * ny);
            m_vAnimation->setStartValue(sy);
            m_vAnimation->setEndValue(ny);
        }
    } else {
        m_vAnimation = nullptr;
    }

    connect(m_animation, &QParallelAnimationGroup::finished, this, &WorksheetView::endAnimation);
    m_animation->start();
}

void WorksheetView::scrollTo(int y)
{
    if (!verticalScrollBar())
        return;

    qreal dy = y - verticalScrollBar()->value();
    scrollBy(dy);
}


bool WorksheetView::isVisible(const QRectF& sceneRect) const
{
    const qreal w = viewport()->width();
    const qreal h = viewport()->height();

    QRectF rect(m_scale*sceneRect.topLeft(), m_scale*sceneRect.size());

    qreal x,y;
    if (m_animation) {
        x = m_hAnimation->endValue().toReal();
        y = m_vAnimation->endValue().toReal();
    } else {
        if (horizontalScrollBar())
            x = horizontalScrollBar()->value();
        else
            x = 0;
        if (verticalScrollBar())
            y = verticalScrollBar()->value();
        else
            y = 0;
    }

    return QRectF(x,y,w,h).contains(rect);
}

bool WorksheetView::isAtEnd() const
{
    bool atEnd = true;
    if (verticalScrollBar())
        atEnd &= (verticalScrollBar()->value()==verticalScrollBar()->maximum());
    return atEnd;
}

void WorksheetView::scrollToEnd() const
{
    if (verticalScrollBar())
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void WorksheetView::scrollBy(int dy)
{
    if (!verticalScrollBar())
        return;

    int ny = verticalScrollBar()->value() + dy;
    if (ny < 0)
        ny = 0;
    else if (ny > verticalScrollBar()->maximum())
        ny = verticalScrollBar()->maximum();

    int x;
    if (horizontalScrollBar())
        x = horizontalScrollBar()->value();
    else
        x = 0;

    const qreal w = viewport()->width() / m_scale;
    const qreal h = viewport()->height() / m_scale;
    makeVisible(QRectF(x, ny, w, h));
}

void WorksheetView::endAnimation()
{
    if (!m_animation)
        return;

    m_animation->deleteLater();
    m_hAnimation = nullptr;
    m_vAnimation = nullptr;
    m_animation = nullptr;
}

QPoint WorksheetView::viewCursorPos() const
{
    return viewport()->mapFromGlobal(QCursor::pos());
}

QPointF WorksheetView::sceneCursorPos() const
{
    return mapToScene(viewCursorPos());
}

QRectF WorksheetView::viewRect() const
{
    const qreal w = viewport()->width() / m_scale;
    const qreal h = viewport()->height() / m_scale;
    qreal y = verticalScrollBar()->value();
    qreal x = horizontalScrollBar() ? horizontalScrollBar()->value() : 0;
    return QRectF(x, y, w, h);
}

void WorksheetView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateSceneSize();
}

void WorksheetView::focusInEvent(QFocusEvent* event)
{
    QGraphicsView::focusInEvent(event);
    m_worksheet->resumeAnimations();
}

void WorksheetView::focusOutEvent(QFocusEvent* event)
{
    QGraphicsView::focusOutEvent(event);
    if (!scene()->hasFocus())
        m_worksheet->stopAnimations();
}


qreal WorksheetView::scaleFactor() const
{
    return m_scale;
}

void WorksheetView::setScaleFactor(qreal zoom, bool emitSignal)
{
    scale(1/m_scale * zoom, 1/m_scale * zoom);
    m_scale = zoom;
    updateSceneSize();
    if (emitSignal)
        emit scaleFactorChanged(m_scale);
}

void WorksheetView::updateSceneSize()
{
    QSize s = viewport()->size();
    m_worksheet->setViewSize(s.width()/m_scale, s.height()/m_scale, m_scale);
    sendViewRectChange();
}

void WorksheetView::sceneRectChanged(const QRectF& sceneRect) const
{
    Q_UNUSED(sceneRect);
    if (verticalScrollBar())
        connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(sendViewRectChange()), Qt::UniqueConnection);
    if (horizontalScrollBar())
        connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(sendViewRectChange()), Qt::UniqueConnection);
}

void WorksheetView::sendViewRectChange() const
{
    emit viewRectChanged(viewRect());
}

void WorksheetView::zoomIn()
{
    m_scale *= 1.1;
    scale(1.1, 1.1);
    updateSceneSize();
    emit scaleFactorChanged(m_scale);
}

void WorksheetView::zoomOut()
{
    m_scale /= 1.1;
    scale(1/1.1, 1/1.1);
    updateSceneSize();
    emit scaleFactorChanged(m_scale);
}

void WorksheetView::actualSize()
{
    scale (1/m_scale, 1/m_scale);
    m_scale = 1.0;
    updateSceneSize();
    emit scaleFactorChanged(m_scale);
}
