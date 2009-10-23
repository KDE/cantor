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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "animationhandler.h"

#include <QMovie>
#include <QPainter>
#include <kdebug.h>
#include "animation.h"

AnimationHandler::AnimationHandler(QTextDocument *doc)
    : QObject(doc)
{
    m_defaultAnimationHandler = doc->documentLayout()->handlerForObject(QTextFormat::ImageObject);
}

QSizeF AnimationHandler::intrinsicSize(QTextDocument *doc, int posInDoc, const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();
    QString name = imageFormat.name();

    const AnimationHelperItem& anim = format.property(AnimationHandler::MovieProperty).value<AnimationHelperItem>();
    QMovie* movie=anim.movie();

    if (!movie)
        return m_defaultAnimationHandler->intrinsicSize(doc, posInDoc, format);
    else
        return movie->currentImage().size();
}

void AnimationHandler::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDoc, const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();
    QString name = imageFormat.name();
    const AnimationHelperItem& anim = format.property(AnimationHandler::MovieProperty).value<AnimationHelperItem>();
    QMovie* movie=anim.movie();

    if (!movie)
        m_defaultAnimationHandler->drawObject(painter, rect, doc, posInDoc, format);
    else
        painter->drawPixmap(rect.topLeft(), movie->currentPixmap());
}

