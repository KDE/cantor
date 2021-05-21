/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
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
