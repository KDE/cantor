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

#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <QObject>
#include <QTextCursor>
#include <QPointer>
#include <QSharedPointer>

class QMovie;

//Represents an animation placed in a QTextDocument
class Animation : public QObject
{
  Q_OBJECT
  public:
    Animation( QObject* parent=nullptr);
    ~Animation() override;

    void setMovie(QMovie* movie);
    QMovie* movie();

    void setPosition(const QTextCursor& cursor);
    QTextCursor position();

  public Q_SLOTS:
    void movieFrameChanged();

  private:
    QPointer<QMovie> m_movie;
    QTextCursor m_position;
};


//Helper Object used for storing Animations in the TextCharFormat
class AnimationHelperItem
{
  public:
    AnimationHelperItem( );
    AnimationHelperItem( const AnimationHelperItem& other);
    ~AnimationHelperItem();

    QTextCursor position() const;
    void setPosition(const QTextCursor& cursor);

    void setMovie(QMovie* movie);
    QMovie* movie() const;

  private:
    QSharedPointer<Animation> m_animation;
};


Q_DECLARE_METATYPE(AnimationHelperItem)

#endif /* _ANIMATION_H */
