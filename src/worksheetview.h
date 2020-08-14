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

#ifndef WORKSHEETVIEW_H
#define WORKSHEETVIEW_H

#include <QGraphicsView>

class QParallelAnimationGroup;
class QPropertyAnimation;

class Worksheet;

class WorksheetView : public QGraphicsView
{
  Q_OBJECT
public:
    WorksheetView(Worksheet* scene, QWidget* parent);

    void makeVisible(const QRectF& sceneRect);
    bool isVisible(const QRectF& sceneRect) const;
    bool isAtEnd() const;
    void scrollToEnd() const;
    void scrollBy(int dy);
    void scrollTo(int y);

    QPoint viewCursorPos() const;
    QPointF sceneCursorPos() const;
    QRectF viewRect() const;
    qreal scaleFactor() const;
    void setScaleFactor(qreal scale, bool emitSignal = true);
    void updateSceneSize();

Q_SIGNALS:
    void viewRectChanged(QRectF rect) const;
    void scaleFactorChanged(double scale);

public Q_SLOTS:
    void zoomIn();
    void zoomOut();
    void actualSize();
    void endAnimation();
    void sceneRectChanged(const QRectF& sceneRect) const;
    void sendViewRectChange() const;

private:
    void resizeEvent(QResizeEvent*) override;

    qreal m_scale;
    QParallelAnimationGroup* m_animation;
    QPropertyAnimation* m_hAnimation;
    QPropertyAnimation* m_vAnimation;
    Worksheet* m_worksheet;
};

#endif //WORKSHEETVIEW_H
