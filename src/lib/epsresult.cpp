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
using namespace MathematiK;

#include "libspectre/spectre.h"
#include <kdebug.h>
#include <kzip.h>
#include <QImage>

class MathematiK::EpsResultPrivate{
    public:
        KUrl url;
        SpectreDocument* doc;
        SpectreRenderContext* rc;
};


EpsResult::EpsResult(const KUrl& url) : d(new EpsResultPrivate)
{
    d->url=url;
    d->doc=spectre_document_new();
    d->rc=spectre_render_context_new();
    spectre_document_load(d->doc, url.toLocalFile().toUtf8());
}

EpsResult::~EpsResult()
{
    spectre_document_free(d->doc);
    spectre_render_context_free(d->rc);
    delete d;
}

QString EpsResult::toHtml()
{
    return QString("<img src=\"%1\" />").arg(resourceUrl().url());
}

QVariant EpsResult::data()
{
    int w, h;
    double scale=1.8;
    spectre_document_get_page_size(d->doc, &w, &h);
    kDebug()<<"dimension: "<<w<<"x"<<h;
    unsigned char* data;
    int rowLength;

    spectre_render_context_set_scale(d->rc, scale, scale);
    spectre_document_render_full( d->doc, d->rc, &data, &rowLength);

    QImage img(data, w*scale, h*scale, rowLength, QImage::Format_RGB32);

    return img;
}

int EpsResult::type()
{
    return EpsResult::Type;
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

