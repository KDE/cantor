/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
*/

#include "worksheetimageitem.h"
#include "worksheet.h"

#include <QApplication>
#include <QBrush>
#include <QFocusEvent>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace
{
constexpr qreal HandleSize = 8.0;
constexpr qreal MinimumImageSize = 32.0;

enum ResizePosition {
    Top = 0x1,
    Bottom = 0x2,
    Left = 0x4,
    Right = 0x8,
    TopLeft = Top | Left,
    TopRight = Top | Right,
    BottomLeft = Bottom | Left,
    BottomRight = Bottom | Right,
};

struct ResizeGeometry {
    QPointF position;
    QSizeF size;
};

ResizeGeometry resizedGeometry(int position, const QPointF& startPosition, const QSizeF& startSize, const QPointF& delta)
{
    ResizeGeometry geometry{startPosition, startSize};
    if (startSize.width() <= 0.0 || startSize.height() <= 0.0)
        return geometry;

    const bool cornerResize = (position & (Left | Right)) && (position & (Top | Bottom));
    if (cornerResize) {
        const qreal horizontalDirection = (position & Left) ? -1.0 : 1.0;
        const qreal verticalDirection = (position & Top) ? -1.0 : 1.0;
        const QPointF originalDiagonal(horizontalDirection * startSize.width(), verticalDirection * startSize.height());
        const QPointF requestedDiagonal = originalDiagonal + delta;
        const qreal diagonalLengthSquared = QPointF::dotProduct(originalDiagonal, originalDiagonal);
        qreal scale = QPointF::dotProduct(requestedDiagonal, originalDiagonal) / diagonalLengthSquared;
        const qreal minimumScale = qMax(MinimumImageSize / startSize.width(), MinimumImageSize / startSize.height());
        scale = qMax(scale, minimumScale);

        geometry.size = startSize * scale;
        if (position & Left)
            geometry.position.rx() += startSize.width() - geometry.size.width();
        if (position & Top)
            geometry.position.ry() += startSize.height() - geometry.size.height();
        return geometry;
    }

    if (position & Left) {
        const qreal dx = qMin(delta.x(), startSize.width() - MinimumImageSize);
        geometry.position.rx() += dx;
        geometry.size.rwidth() -= dx;
    } else if (position & Right)
        geometry.size.setWidth(qMax(MinimumImageSize, startSize.width() + delta.x()));

    if (position & Top) {
        const qreal dy = qMin(delta.y(), startSize.height() - MinimumImageSize);
        geometry.position.ry() += dy;
        geometry.size.rheight() -= dy;
    } else if (position & Bottom)
        geometry.size.setHeight(qMax(MinimumImageSize, startSize.height() + delta.y()));

    return geometry;
}
}

class WorksheetImageResizeHandle : public QGraphicsRectItem
{
public:
    WorksheetImageResizeHandle(int position, WorksheetImageItem* parent)
        : QGraphicsRectItem(-HandleSize / 2.0, -HandleSize / 2.0, HandleSize, HandleSize, parent)
        , m_position(position)
        , m_parent(parent)
    {
        setAcceptHoverEvents(true);
        setBrush(QApplication::palette().highlight());
        setPen(QPen(QApplication::palette().highlightedText(), 1.0));
        setZValue(1.0);
        hide();
    }

    int position() const
    {
        return m_position;
    }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
    {
        if (m_position == TopLeft || m_position == BottomRight)
            setCursor(Qt::SizeFDiagCursor);
        else if (m_position == TopRight || m_position == BottomLeft)
            setCursor(Qt::SizeBDiagCursor);
        else if (m_position == Top || m_position == Bottom)
            setCursor(Qt::SizeVerCursor);
        else
            setCursor(Qt::SizeHorCursor);

        QGraphicsRectItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
    {
        unsetCursor();
        QGraphicsRectItem::hoverLeaveEvent(event);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton) {
            QGraphicsRectItem::mousePressEvent(event);
            return;
        }

        m_parent->setFocus(Qt::MouseFocusReason);
        m_parent->beginResize(m_position, event->scenePos());
        event->accept();
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (!(event->buttons() & Qt::LeftButton)) {
            QGraphicsRectItem::mouseMoveEvent(event);
            return;
        }

        m_parent->continueResize(event->scenePos());
        event->accept();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton) {
            QGraphicsRectItem::mouseReleaseEvent(event);
            return;
        }

        m_parent->endResize();
        event->accept();
    }

private:
    int m_position;
    WorksheetImageItem* m_parent;
};

WorksheetImageItem::WorksheetImageItem(QGraphicsObject* parent)
    : QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    connect(this, SIGNAL(menuCreated(QMenu*,QPointF)), parent,
            SLOT(populateMenu(QMenu*,QPointF)), Qt::DirectConnection);
}

WorksheetImageItem::~WorksheetImageItem()
{
    if (worksheet())
        worksheet()->removeRequestedWidth(this);
}

int WorksheetImageItem::type() const
{
    return Type;
}

bool WorksheetImageItem::imageIsValid()
{
    return !m_pixmap.isNull();
}

qreal WorksheetImageItem::setGeometry(qreal x, qreal y, qreal w, bool centered)
{
    if (width() <= w && centered)
        setPos(x + w/2 - width()/2, y);
    else
        setPos(x, y);

    worksheet()->setRequestedWidth(this, scenePos().x() + width());

    return height();
}

qreal WorksheetImageItem::height() const
{
    return m_size.height();
}

qreal WorksheetImageItem::width() const
{
    return m_size.width();
}

QSizeF WorksheetImageItem::size()
{
    return m_size;
}

void WorksheetImageItem::setSize(QSizeF size)
{
    if (size == m_size)
        return;

    prepareGeometryChange();
    m_size = size;
    updateResizeHandlePositions();
    const qreal width = scenePos().x() + size.width();
    if (worksheet())
        worksheet()->setRequestedWidth(this, width);
    update();
}

QSize WorksheetImageItem::imageSize()
{
    return m_pixmap.size();
}

QRectF WorksheetImageItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), m_size);
}

void WorksheetImageItem::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawPixmap(QRectF(QPointF(0,0), m_size), m_pixmap,
                        m_pixmap.rect());
    if (hasFocus())
    {
        painter->setPen(Qt::DashLine);
        painter->drawRect(0, 0, width(), height());
    }
}

void WorksheetImageItem::setPdf(const QUrl& url)
{
    const QImage img = worksheet()->renderer()->renderToImage(url, &m_size);
    m_pixmap = QPixmap::fromImage(img.convertToFormat(QImage::Format_ARGB32));
}

void WorksheetImageItem::setImage(const QImage& img)
{
    m_pixmap = QPixmap::fromImage(img);
    setSize(m_pixmap.size());
}

void WorksheetImageItem::setImage(const QImage& img, QSize displaySize)
{
    m_pixmap = QPixmap::fromImage(img);
    setSize(displaySize);
}

void WorksheetImageItem::setResizable(bool resizable)
{
    if (m_resizable == resizable)
        return;

    m_resizable = resizable;
    if (m_resizable && m_resizeHandles.isEmpty()) {
        m_resizeHandles.append(new WorksheetImageResizeHandle(TopLeft, this));
        m_resizeHandles.append(new WorksheetImageResizeHandle(Top, this));
        m_resizeHandles.append(new WorksheetImageResizeHandle(TopRight, this));
        m_resizeHandles.append(new WorksheetImageResizeHandle(Right, this));
        m_resizeHandles.append(new WorksheetImageResizeHandle(BottomRight, this));
        m_resizeHandles.append(new WorksheetImageResizeHandle(Bottom, this));
        m_resizeHandles.append(new WorksheetImageResizeHandle(BottomLeft, this));
        m_resizeHandles.append(new WorksheetImageResizeHandle(Left, this));
        updateResizeHandlePositions();
    }

    for (auto* handle : m_resizeHandles)
        handle->setVisible(m_resizable && hasFocus());
}

void WorksheetImageItem::setPixmap(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
}

QPixmap WorksheetImageItem::pixmap() const
{
    return m_pixmap;
}

void WorksheetImageItem::populateMenu(QMenu* menu, QPointF pos)
{
    Q_EMIT menuCreated(menu, mapToParent(pos));
}

void WorksheetImageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    auto* menu = worksheet()->createContextMenu();
    populateMenu(menu, event->pos());
    menu->popup(event->screenPos());
}

void WorksheetImageItem::focusInEvent(QFocusEvent* event)
{
    for (auto* handle : m_resizeHandles)
        handle->setVisible(m_resizable);
    update();
    QGraphicsObject::focusInEvent(event);
}

void WorksheetImageItem::focusOutEvent(QFocusEvent* event)
{
    if (!m_resizePosition) {
        for (auto* handle : m_resizeHandles)
            handle->hide();
    }
    update();
    QGraphicsObject::focusOutEvent(event);
}

void WorksheetImageItem::beginResize(int position, const QPointF& scenePos)
{
    m_resizePosition = position;
    m_resizeStartScenePos = scenePos;
    m_resizeStartItemPos = pos();
    m_resizeStartSize = m_size;
    Q_EMIT resizeStarted(position == TopLeft || position == TopRight);
}

void WorksheetImageItem::continueResize(const QPointF& scenePos)
{
    if (!m_resizePosition)
        return;

    const QPointF delta = parentItem()
        ? parentItem()->mapFromScene(scenePos) - parentItem()->mapFromScene(m_resizeStartScenePos)
        : scenePos - m_resizeStartScenePos;
    const auto geometry = resizedGeometry(m_resizePosition, m_resizeStartItemPos, m_resizeStartSize, delta);

    if (geometry.position == pos() && geometry.size == m_size)
        return;

    setPos(geometry.position);
    setSize(geometry.size);
    Q_EMIT resizePreviewChanged(geometry.size);
}

void WorksheetImageItem::endResize()
{
    if (!m_resizePosition)
        return;

    m_resizePosition = 0;
    Q_EMIT resizeFinished(m_size);

    if (!hasFocus()) {
        for (auto* handle : m_resizeHandles)
            handle->hide();
    }
}

void WorksheetImageItem::updateResizeHandlePositions()
{
    for (auto* handle : m_resizeHandles) {
        switch (handle->position()) {
        case TopLeft:
            handle->setPos(0.0, 0.0);
            break;
        case Top:
            handle->setPos(m_size.width() / 2.0, 0.0);
            break;
        case TopRight:
            handle->setPos(m_size.width(), 0.0);
            break;
        case Right:
            handle->setPos(m_size.width(), m_size.height() / 2.0);
            break;
        case BottomRight:
            handle->setPos(m_size.width(), m_size.height());
            break;
        case Bottom:
            handle->setPos(m_size.width() / 2.0, m_size.height());
            break;
        case BottomLeft:
            handle->setPos(0.0, m_size.height());
            break;
        case Left:
            handle->setPos(0.0, m_size.height() / 2.0);
            break;
        }
    }
}

Worksheet* WorksheetImageItem::worksheet()
{
    return qobject_cast<Worksheet*>(scene());
}
