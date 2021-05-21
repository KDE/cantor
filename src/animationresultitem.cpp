/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "animationresultitem.h"
#include "commandentry.h"
#include "lib/result.h"
#include "lib/animationresult.h"

#include <QDebug>
#include <QFileDialog>
#include <QMovie>

#include <KLocalizedString>

AnimationResultItem::AnimationResultItem(QGraphicsObject* parent, Cantor::Result* result)
    : WorksheetImageItem(parent), ResultItem(result), m_height(0), m_movie(nullptr)
{
    update();
}

double AnimationResultItem::setGeometry(double x, double y, double w)
{
    Q_UNUSED(w);
    setPos(x,y);

    return m_height;
}

void AnimationResultItem::populateMenu(QMenu* menu, QPointF pos)
{
    ResultItem::addCommonActions(this, menu);

    menu->addSeparator();
    if (m_movie) {
        if (m_movie->state() == QMovie::Running)
            menu->addAction(QIcon::fromTheme(QLatin1String("media-playback-pause")), i18n("Pause"),
                            this, SLOT(pauseMovie()));
        else
            menu->addAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Start"),
                            m_movie, SLOT(start()));
        if (m_movie->state() == QMovie::Running ||
            m_movie->state() == QMovie::Paused)
            menu->addAction(QIcon::fromTheme(QLatin1String("media-playback-stop")), i18n("Stop"),
                            this, SLOT(stopMovie()));
    }
    qDebug() << "populate Menu";
    emit menuCreated(menu, mapToParent(pos));
}

void AnimationResultItem::update()
{
    Q_ASSERT(m_result->type() == Cantor::AnimationResult::Type);
    QMovie* mov;
    switch(m_result->type()) {
    case Cantor::AnimationResult::Type:
        mov = static_cast<QMovie*>(m_result->data().value<QObject*>());
        setMovie(mov);
        break;
    default:
        break;
    }
}

QRectF AnimationResultItem::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}

double AnimationResultItem::width() const
{
    return WorksheetImageItem::width();
}

double AnimationResultItem::height() const
{
    return WorksheetImageItem::height();
}


void AnimationResultItem::setMovie(QMovie* movie)
{
    if (m_movie) {
        m_movie->disconnect(this, SLOT(updateFrame()));
        m_movie->disconnect(this, SLOT(updateSize()));
    }
    m_movie = movie;
    m_height = 0;
    if (m_movie) {
        connect(m_movie, &QMovie::frameChanged, this, &AnimationResultItem::updateFrame);
        connect(m_movie, &QMovie::resized, this, &AnimationResultItem::updateSize);
        m_movie->start();
    }
}

void AnimationResultItem::updateFrame()
{
    setImage(m_movie->currentImage());
    worksheet()->update(mapRectToScene(boundingRect()));
}

void AnimationResultItem::updateSize(QSize size)
{
    if (m_height != size.height()) {
        m_height = size.height();
        emit sizeChanged();
    }
}

void AnimationResultItem::saveResult()
{
    Cantor::Result* res = result();
    const QString& filename=QFileDialog::getSaveFileName(worksheet()->worksheetView(), i18n("Save result"), QString(), res->mimeType());
    qDebug()<<"saving result to "<<filename;
    res->save(filename);
}

void AnimationResultItem::stopMovie()
{
    if (m_movie) {
        m_movie->stop();
        m_movie->jumpToFrame(0);
        worksheet()->update(mapRectToScene(boundingRect()));
    }
}

void AnimationResultItem::pauseMovie()
{
    if (m_movie)
        m_movie->setPaused(true);
}

void AnimationResultItem::deleteLater()
{
    WorksheetImageItem::deleteLater();
}
