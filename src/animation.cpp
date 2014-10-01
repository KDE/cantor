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
    disconnect(0, 0, this, SLOT(movieFrameChanged()));
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
        disconnect(m_movie, SIGNAL(frameChanged(int)), this, SLOT(movieFrameChanged()));
    }
}



AnimationHelperItem::AnimationHelperItem( ) : m_animation(new Animation())
{
}

AnimationHelperItem::AnimationHelperItem(const AnimationHelperItem& other)
{
    m_animation=other.m_animation;
}

AnimationHelperItem::~AnimationHelperItem()
{
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

#include "animation.moc"
