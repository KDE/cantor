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
    Copyright (C) 2011 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include <QPainter>

#include "formulatextobject.h"
#include <kurl.h>
#include <kdebug.h>

QSizeF FormulaTextObject::intrinsicSize(QTextDocument * doc, int /*posInDocument*/,
                                        const QTextFormat &format)
{
    KUrl url=qVariantValue<KUrl>(format.property(ResourceUrl));
    //kDebug()<<"looking for: "<<url;
    QImage bufferedImage = qVariantValue<QImage>(doc->resource(QTextDocument::ImageResource, url));
    QSize size = bufferedImage.size();

    //kDebug()<<"size: "<<size;
    return QSizeF(size);
}

void FormulaTextObject::drawObject(QPainter *painter, const QRectF &rect,
                                   QTextDocument * doc, int /*posInDocument*/,
                                   const QTextFormat &format)
{
    KUrl url=qVariantValue<KUrl>(format.property(ResourceUrl));
    QImage bufferedImage = qVariantValue<QImage>(doc->resource(QTextDocument::ImageResource, url));

    painter->drawImage(rect, bufferedImage);
}

