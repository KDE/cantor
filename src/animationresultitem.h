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

