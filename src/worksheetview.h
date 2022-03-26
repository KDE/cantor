/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
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
    WorksheetView(Worksheet*, QWidget*);

    void makeVisible(const QRectF&);
    bool isVisible(const QRectF&) const;
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
    void viewRectChanged(QRectF) const;
    void scaleFactorChanged(double scale);

public Q_SLOTS:
    void zoomIn();
    void zoomOut();
    void actualSize();
    void endAnimation();
    void sceneRectChanged(const QRectF&) const;
    void sendViewRectChange() const;

private:
    void resizeEvent(QResizeEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    void wheelEvent(QWheelEvent*) override;

    void zoom(int);
    void scalingTime();
    void animFinished();

    qreal m_scale = 1.;
    int m_numScheduledScalings{0};
    QParallelAnimationGroup* m_animation{nullptr};
    QPropertyAnimation* m_hAnimation{nullptr};
    QPropertyAnimation* m_vAnimation{nullptr};
    Worksheet* m_worksheet;
};

#endif //WORKSHEETVIEW_H
