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

#ifndef _ANIMATIONHANDLER_H
#define _ANIMATIONHANDLER_H

#include <QTextObjectInterface>
#include <QTextDocument>

class AnimationHandler : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    enum {MovieProperty = QTextFormat::UserProperty+10};
    AnimationHandler(QTextDocument *doc);

    QSizeF intrinsicSize(QTextDocument *doc, int posInDoc, const QTextFormat &format);

    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDoc, const QTextFormat &format);

private:
    QTextObjectInterface *m_defaultAnimationHandler;
};

#endif /* _ANIMATIONHANDLER_H */
