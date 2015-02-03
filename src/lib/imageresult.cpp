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
using namespace Cantor;

#include <QImage>
#include <QImageWriter>
#include <KZip>
#include <QDebug>

class Cantor::ImageResultPrivate
{
  public:
    ImageResultPrivate()
    {

    }

    QUrl url;
    QImage img;
    QString alt;
};

ImageResult::ImageResult(const QUrl &url, const QString& alt) :  d(new ImageResultPrivate)
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
    return QString::fromLatin1("<img src=\"%1\" alt=\"%2\"/>").arg(d->url.toLocalFile(), d->alt);
}

QString ImageResult::toLatex()
{
    return QString::fromLatin1(" \\begin{center} \n \\includegraphics[width=12cm]{%1} \n \\end{center}").arg(d->url.fileName());
}

QVariant ImageResult::data()
{
    if(d->img.isNull())
        d->img.load(d->url.toLocalFile());

    return QVariant(d->img);
}

QUrl ImageResult::url()
{
    return d->url;
}

int ImageResult::type()
{
    return ImageResult::Type;
}

QString ImageResult::mimeType()
{
    const QList<QByteArray> formats=QImageWriter::supportedImageFormats();
    QString mimetype;
    foreach(const QByteArray& format, formats)
    {
        mimetype+=QLatin1String("image/"+format.toLower()+' ');
    }
    qDebug()<<"type: "<<mimetype;

    return mimetype;
}

QDomElement ImageResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving imageresult "<<toHtml();
    QDomElement e=doc.createElement(QLatin1String("Result"));
    e.setAttribute(QLatin1String("type"), QLatin1String("image"));
    e.setAttribute(QLatin1String("filename"), d->url.fileName());
    qDebug()<<"done";

    return e;
}

void ImageResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

void ImageResult::save(const QString& filename)
{
    //load into memory and let Qt save it, instead of just copying d->url
    //to give possibility to convert file format
    QImage img=data().value<QImage>();

    img.save(filename);
}

