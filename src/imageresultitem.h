/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef IMAGERESULTITEM_H
#define IMAGERESULTITEM_H

#include "resultitem.h"
#include "worksheetimageitem.h"

class CommandEntry;
class EpsRenderer;

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

  protected Q_SLOTS:
    void saveResult();
};

#endif // IMAGERESULTITEM_H
