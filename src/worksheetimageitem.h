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

#ifndef WORKSHEETIMAGEITEM_H
#define WORKSHEETIMAGEITEM_H

#include <QPixmap>
#include <QGraphicsObject>

class Worksheet;
class QImage;
class QGraphicsSceneContextMenuEvent;
class QMenu;

class WorksheetImageItem : public QGraphicsObject
{
  Q_OBJECT
  public:
    explicit WorksheetImageItem(QGraphicsObject* parent);
    ~WorksheetImageItem() override;

    enum {Type = UserType + 101};

    int type() const override;

    bool imageIsValid();

    virtual qreal setGeometry(qreal x, qreal y, qreal w, bool centered=false);

    qreal height() const;
    qreal width() const;
    QSizeF size();
    void setSize(QSizeF size);
    QSize imageSize();

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    void setEps(const QUrl &url);
    void setImage(QImage img);
    void setImage(QImage img, QSize displaySize);
    void setPixmap(QPixmap pixmap);
    QPixmap pixmap() const;

    virtual void populateMenu(QMenu* menu, QPointF pos);
    Worksheet* worksheet();

  Q_SIGNALS:
    void sizeChanged();
    void menuCreated(QMenu*, QPointF);

  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;

  private:
    QPixmap m_pixmap;
    QSizeF m_size;
};

#endif //WORKSHEETIMAGEITEM_H
