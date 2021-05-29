/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef WORKSHEETTOOLBUTTON_H
#define WORKSHEETTOOLBUTTON_H

#include <QGraphicsObject>
#include <QPixmap>

#include <QIcon>

class WorksheetToolButton : public QGraphicsObject
{
  Q_OBJECT
  public:
    explicit WorksheetToolButton(QGraphicsItem* parent);
    ~WorksheetToolButton() override = default;

    void setIcon(const QIcon& icon);

    qreal width();
    qreal height();
    QRectF boundingRect() const override;
    void setIconScale(qreal scale);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

  Q_SIGNALS:
    void clicked();
    void pressed();

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

  private:
    QSize m_size;
    QPixmap m_pixmap;
    QIcon m_icon;
    qreal m_scale;
};

#endif //WORKSHEETTOOLBUTTON_H
