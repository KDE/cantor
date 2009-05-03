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

#include "imageresult.h"
using namespace MathematiK;

#include <QImage>
#include <kzip.h>
#include <kdebug.h>

class MathematiK::ImageResultPrivate
{
  public:
    ImageResultPrivate()
    {

    }

    KUrl url;
    QImage img;
    QString alt;
};

ImageResult::ImageResult(const KUrl& url, const QString& alt) :  d(new ImageResultPrivate)
{
    d->url=url;
    d->alt=alt;
}

ImageResult::~ImageResult()
{
    delete d;
}

QString ImageResult::toHtml()
{
    return QString("<img src=\"%1\" alt=\"%2\"/>").arg(d->url.toLocalFile(), d->alt);
}

QVariant ImageResult::data()
{
    if(d->img.isNull())
        d->img.load(d->url.toLocalFile());

    return QVariant(d->img);
}

int ImageResult::type()
{
    return ImageResult::Type;
}

QDomElement ImageResult::toXml(QDomDocument& doc)
{
    kDebug()<<"saving imageresult "<<toHtml();
    QDomElement e=doc.createElement("Result");
    e.setAttribute("type", "image");
    e.setAttribute("filename", d->url.fileName());
    kDebug()<<"done";

    return e;
}

void ImageResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

