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

#include "epsresult.h"
using namespace Cantor;

#include <kdebug.h>
#include <kzip.h>
#include <kio/job.h>

class Cantor::EpsResultPrivate{
    public:
        KUrl url;
};


EpsResult::EpsResult(const KUrl& url) : d(new EpsResultPrivate)
{
    d->url=url;
}

EpsResult::~EpsResult()
{
    delete d;
}

QString EpsResult::toHtml()
{
    return QString("<img src=\"%1\" />").arg(d->url.url());
}

QVariant EpsResult::data()
{
    return QVariant(d->url);
}

int EpsResult::type()
{
    return EpsResult::Type;
}

QString EpsResult::mimeType()
{
    return "image/x-eps";
}

QDomElement EpsResult::toXml(QDomDocument& doc)
{
    kDebug()<<"saving imageresult "<<toHtml();
    QDomElement e=doc.createElement("Result");
    e.setAttribute("type", "image");
    e.setAttribute("filename", d->url.fileName());
    kDebug()<<"done";

    return e;
}

void EpsResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

void EpsResult::save(const QString& filename)
{
    //just copy over the eps file..
    KIO::file_copy(d->url, KUrl(filename), -1, KIO::HideProgressInfo);
}
