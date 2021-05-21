/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
    explicit Animation( QObject* parent=nullptr);
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
    ~AnimationHelperItem() = default;

    QTextCursor position() const;
    void setPosition(const QTextCursor& cursor);

    void setMovie(QMovie* movie);
    QMovie* movie() const;

  private:
    QSharedPointer<Animation> m_animation;
};


Q_DECLARE_METATYPE(AnimationHelperItem)

#endif /* _ANIMATION_H */
