/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef IMAGERESULTITEM_H
#define IMAGERESULTITEM_H

#include "resultitem.h"
#include "worksheetimageitem.h"

#include <QTimer>

class CommandEntry;
class QGraphicsSceneMouseEvent;

class ImageResultItem : public WorksheetImageItem, public ResultItem
{
  Q_OBJECT
  public:
    explicit ImageResultItem(QGraphicsObject* parent, Cantor::Result* result);
    ~ImageResultItem() override = default;

    using WorksheetImageItem::setGeometry;
    double setGeometry(double x, double y, double w) override;
    void populateMenu(QMenu* menu, QPointF pos) override;

    void update() override;

    QRectF boundingRect() const override;
    double width() const override;
    double height() const override;

    void deleteLater() override;

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  protected Q_SLOTS:
    void saveResult();

  private:
    void beginResizePreview(bool fromTopCorner);
    void updateResizePreview(const QSizeF& size);
    void flushResizePreview();
    void applyDisplaySize(const QSizeF& size);
    void renderPdf(const QSize& displaySize = QSize());

    qreal m_parentZValue{0.0};
    qreal m_zValue{0.0};
    qreal m_parentHeightBeforeResize{0.0};
    qreal m_imageHeightBeforeResize{0.0};
    QSizeF m_pendingPreviewSize;
    QTimer m_resizePreviewTimer;
    bool m_resizePreviewDirty{false};
    bool m_resizePreviewActive{false};
    bool m_moveFollowingEntriesDuringResize{true};
};

#endif // IMAGERESULTITEM_H
