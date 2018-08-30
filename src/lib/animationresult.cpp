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

#include "animationresult.h"
using namespace Cantor;

#include <QImage>
#include <QImageWriter>
#include <KZip>
#include <QMimeDatabase>
#include <QDebug>
#include <KIO/Job>
#include <QMovie>

class Cantor::AnimationResultPrivate
{
  public:
    AnimationResultPrivate() = default;

    QUrl url;
    QMovie* movie;
    QString alt;
};

AnimationResult::AnimationResult(const QUrl &url, const QString& alt ) : d(new AnimationResultPrivate)
{
    d->url=url;
    d->alt=alt;
    d->movie=new QMovie();
    d->movie->setFileName(url.toLocalFile());
}


AnimationResult::~AnimationResult()
{
    delete d->movie;
    delete d;
}

QString AnimationResult::toHtml()
{
    return QString::fromLatin1("<img src=\"%1\" alt=\"%2\"/>").arg(d->url.toLocalFile(), d->alt);
}

QVariant AnimationResult::data()
{
    return QVariant::fromValue(static_cast<QObject*>(d->movie));
}

QUrl AnimationResult::url()
{
    return d->url;
}

int AnimationResult::type()
{
    return AnimationResult::Type;
}

QString AnimationResult::mimeType()
{
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForUrl(d->url);

    return type.name();
}

QDomElement AnimationResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving imageresult "<<toHtml();
    QDomElement e=doc.createElement(QLatin1String("Result"));
    e.setAttribute(QLatin1String("type"), QLatin1String("animation"));
    e.setAttribute(QLatin1String("filename"), d->url.fileName());
    qDebug()<<"done";

    return e;
}

void AnimationResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

void AnimationResult::save(const QString& filename)
{
    //just copy over the file..
    KIO::file_copy(d->url, QUrl::fromLocalFile(filename), -1, KIO::HideProgressInfo);
}
