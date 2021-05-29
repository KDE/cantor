/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "animation.h"

#include <QMovie>
#include <QDebug>

Animation::Animation(QObject* parent) : QObject(parent)
{

}

Animation::~Animation()
{
    if(m_movie)
        m_movie->stop();
}

void Animation::setMovie(QMovie* movie)
{
    disconnect(nullptr, nullptr, this, SLOT(movieFrameChanged()));
    m_movie=movie;
    connect(movie, SIGNAL(frameChanged(int)), this, SLOT(movieFrameChanged()));
}

QMovie* Animation::movie()
{
    return m_movie;
}

void Animation::setPosition(const QTextCursor& cursor)
{
    m_position=cursor;
}

QTextCursor Animation::position()
{
    return m_position;
}

void Animation::movieFrameChanged()
{
    QTextCursor cursor = m_position;
    cursor.setPosition(m_position.position()+1, QTextCursor::KeepAnchor);

    const QString& text=cursor.selectedText();

    if (text==QString(QChar::ObjectReplacementCharacter)) {
        // update a bogus property, which will trigger a paint
        QTextCharFormat format2;
        format2.setProperty(QTextFormat::UserFormat + 2, m_movie->currentFrameNumber());
        cursor.mergeCharFormat(format2);
    } else {
        // we got removed from the document
        qDebug()<<"animation got removed";
        disconnect(m_movie.data(), &QMovie::frameChanged, this, &Animation::movieFrameChanged);
    }
}



AnimationHelperItem::AnimationHelperItem( ) : m_animation(new Animation())
{
}

AnimationHelperItem::AnimationHelperItem(const AnimationHelperItem& other)
{
    m_animation=other.m_animation;
}

void AnimationHelperItem::setPosition(const QTextCursor& cursor)
{
    m_animation->setPosition(cursor);
}

QTextCursor AnimationHelperItem::position() const
{
    return m_animation->position();
}

void AnimationHelperItem::setMovie(QMovie* movie)
{
    m_animation->setMovie(movie);
}

QMovie* AnimationHelperItem::movie() const
{
    return m_animation->movie();
}


