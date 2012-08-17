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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "epsrenderer.h"

#include "config-cantor.h"

#ifdef LIBSPECTRE_FOUND
  #include "libspectre/spectre.h"
#endif

#include <kdebug.h>

EpsRenderer::EpsRenderer() : m_scale(1), m_useHighRes(false)
{
}

EpsRenderer::~EpsRenderer()
{}

void EpsRenderer::setScale(double scale)
{
    m_scale = scale;
}

double EpsRenderer::scale()
{
    return m_scale;
}

void EpsRenderer::useHighResolution(bool b)
{
    m_useHighRes = b;
}

QTextImageFormat EpsRenderer::render(QTextDocument *document, const KUrl& url)
{
    QTextImageFormat epsCharFormat;

    QSizeF s = renderToResource(document, url);

    KUrl internal = url;
    internal.setProtocol("internal");
    if(s.isValid())
    {
        epsCharFormat.setName(internal.url());
        epsCharFormat.setWidth(s.width());
        epsCharFormat.setHeight(s.height());
    }

    return epsCharFormat;
}

QTextImageFormat EpsRenderer::render(QTextDocument *document,
                                     const Cantor::LatexRenderer* latex)
{
    QTextImageFormat format = render(document, latex->imagePath());

    if (!format.name().isEmpty()) {
        format.setProperty(CantorFormula, latex->method());
        format.setProperty(ImagePath, latex->imagePath());
        format.setProperty(Code, latex->latexCode());
    }

    return format;
}

QSizeF EpsRenderer::renderToResource(QTextDocument *document, const KUrl& url)
{
    QSizeF size;
    QImage img = renderToImage(url, &size);

    KUrl internal = url;
    internal.setProtocol("internal");
    kDebug() << internal;
    document->addResource(QTextDocument::ImageResource, internal, QVariant(img) );
    return size;
}

QImage EpsRenderer::renderToImage(const KUrl& url, QSizeF* size)
{
#ifdef LIBSPECTRE_FOUND
    SpectreDocument* doc = spectre_document_new();
    SpectreRenderContext* rc = spectre_render_context_new();

    kDebug() << "rendering eps file: " << url;

    spectre_document_load(doc, url.toLocalFile().toUtf8());

    int wdoc, hdoc;
    qreal w, h;
    double scale;
    spectre_document_get_page_size(doc, &wdoc, &hdoc);
    if(m_useHighRes) {
        scale=1.2*4.0; //1.2 scaling factor, to make it look nice, 4x for high resolution
        w = 1.2 * wdoc;
        h = 1.2 * hdoc;
    } else {
        scale=1.8*m_scale;
        w = 1.8 * wdoc;
        h = 1.8 * hdoc;
    }

    kDebug()<<"scale: "<<scale;

    kDebug()<<"dimension: "<<w<<"x"<<h;
    unsigned char* data;
    int rowLength;

    spectre_render_context_set_scale(rc, scale, scale);
    spectre_document_render_full( doc, rc, &data, &rowLength);

    QImage img(data, wdoc*scale, hdoc*scale, rowLength, QImage::Format_RGB32);
    spectre_document_free(doc);
    spectre_render_context_free(rc);

    if (size)
        *size = QSizeF(w,h);
    return img;
#else
    return QImage();
#endif
}
