/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef ANIMATIONRESULTITEM_H
#define ANIMATIONRESULTITEM_H

#include "resultitem.h"
#include "worksheetimageitem.h"

class QMovie;

class CommandEntry;
class WorksheetEntry;

class AnimationResultItem : public WorksheetImageItem, public ResultItem
{
  Q_OBJECT

  public:
    explicit AnimationResultItem(QGraphicsObject*, Cantor::Result*);
    ~AnimationResultItem() override = default;

    using WorksheetImageItem::setGeometry;
    double setGeometry(double x, double y, double w) override;
    void populateMenu(QMenu*, QPointF) override;

    void update() override;

    void deleteLater() override;

    QRectF boundingRect() const override;
    double width() const override;
    double height() const override;

  protected Q_SLOTS:
    void saveResult();
    void stopMovie();
    void pauseMovie();

  private:
    void setMovie(QMovie*);

  private Q_SLOTS:
    void updateFrame();
    void updateSize(QSize);

  private:
    double m_height;
    QMovie* m_movie;
};

#endif //ANIMATIONRESULTITEM_H

