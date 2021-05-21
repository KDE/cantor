/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
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
