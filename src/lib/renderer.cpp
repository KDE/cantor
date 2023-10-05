/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "renderer.h"

#include <QUuid>
#include <QDebug>
#include <QMutex>

#include <config-cantorlib.h>

#include <poppler-qt6.h>
#ifdef LIBSPECTRE_FOUND
  #include "libspectre/spectre.h"
#endif


using namespace Cantor;

// We need this, because poppler-qt5 not threadsafe before 0.73.0 and 0.73.0 is too new
// and not common widespread in repositories
static QMutex popplerMutex;

class Cantor::RendererPrivate{
  public:
    double scale{1};
    bool useHighRes{false};
};

Renderer::Renderer() : d(new RendererPrivate())
{
}

Renderer::~Renderer()
{
    delete d;
}

void Renderer::setScale(qreal scale)
{
    d->scale = scale;
}

qreal Renderer::scale()
{
    return d->scale;
}

void Renderer::useHighResolution(bool b)
{
    d->useHighRes = b;
}

QTextImageFormat Renderer::render(QTextDocument *document, Method method, const QUrl &url, const QString& uuid)
{
    QTextImageFormat format;

    QUrl internal;
    internal.setScheme(QLatin1String("internal"));
    internal.setPath(uuid);

    QSizeF s = renderToResource(document, method, url, internal);

    if(s.isValid())
    {
        format.setName(internal.url());
        format.setWidth(s.width());
        format.setHeight(s.height());
    }

    return format;
}

QTextImageFormat Renderer::render(QTextDocument *document, const Cantor::LatexRenderer* latex)
{
    QTextImageFormat format = render(document, Method::EPS, QUrl::fromLocalFile(latex->imagePath()), latex->uuid());

    if (!format.name().isEmpty()) {
        format.setProperty(CantorFormula, latex->method());
        format.setProperty(ImagePath, latex->imagePath());
        format.setProperty(Code, latex->latexCode());
    }

    return format;
}

QSizeF Renderer::renderToResource(QTextDocument *document, Method method, const QUrl &url, const QUrl& internal)
{
    QSizeF size;
    QImage img = renderToImage(url, method, &size);

    qDebug() << internal;
    document->addResource(QTextDocument::ImageResource, internal, QVariant(img) );
    return size;
}

QImage Renderer::epsRenderToImage(const QUrl& url, double scale, bool useHighRes, QSizeF* size, QString* errorReason)
{
#ifdef LIBSPECTRE_FOUND
    SpectreDocument* doc = spectre_document_new();
    SpectreRenderContext* rc = spectre_render_context_new();

    qDebug() << "rendering eps file: " << url;
    QByteArray local_file = url.toLocalFile().toUtf8();
    spectre_document_load(doc, local_file.data());

    bool isEps = spectre_document_is_eps(doc);
    if (!isEps)
    {
        if (errorReason)
            *errorReason = QString::fromLatin1("Error: spectre document is not eps! It means, that url is invalid");
        return QImage();
    }

    int wdoc, hdoc;
    qreal w, h;
    double realScale;
    spectre_document_get_page_size(doc, &wdoc, &hdoc);
    if(useHighRes) {
        realScale = 1.2*4.0; //1.2 scaling factor, to make it look nice, 4x for high resolution
        w = 1.2 * wdoc;
        h = 1.2 * hdoc;
    } else {
        realScale=1.8*scale;
        w = 1.8 * wdoc;
        h = 1.8 * hdoc;
    }

    qDebug()<<"scale: "<<realScale;

    qDebug()<<"dimension: "<<w<<"x"<<h;
    unsigned char* data;
    int rowLength;

    spectre_render_context_set_scale(rc, realScale, realScale);
    spectre_document_render_full( doc, rc, &data, &rowLength);

    QImage img(data, wdoc*realScale, hdoc*realScale, rowLength, QImage::Format_RGB32);
    spectre_document_free(doc);
    spectre_render_context_free(rc);
    img = img.convertToFormat(QImage::Format_ARGB32);

    if (size)
        *size = QSizeF(w,h);
    return img;
#else
    if (errorReason)
        *errorReason = QString::fromLatin1("Render Eps on Cantor without eps support (libspectre)!");

    Q_UNUSED(url);
    Q_UNUSED(scale);
    Q_UNUSED(useHighRes);
    Q_UNUSED(size);
    return QImage();
#endif
}

QImage Renderer::pdfRenderToImage(const QUrl& url, double scale, bool highResolution, QSizeF* size, QString* errorReason)
{
    popplerMutex.lock();
    auto document = Poppler::Document::load(url.toLocalFile());
    popplerMutex.unlock();
    if (document == nullptr)
    {
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library have failed to open file % as pdf").arg(url.toLocalFile());
        return QImage();
    }

    auto pdfPage = document->page(0);
    if (pdfPage == nullptr) {
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library failed to access first page of %1 document").arg(url.toLocalFile());

        return QImage();
    }

    QSize pageSize = pdfPage->pageSize();

    double realScale = 1.7 * 1.8;
    qreal w = 1.7 * pageSize.width();
    qreal h = 1.7 * pageSize.height();
    if(highResolution)
        realScale *= 5;
    else
        realScale *= scale;


    QImage image = pdfPage->renderToImage(72.0*realScale, 72.0*realScale);

    popplerMutex.lock();
    popplerMutex.unlock();

    if (image.isNull())
    {
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library failed to render pdf %1 to image").arg(url.toLocalFile());

        return image;
    }

    // Resize with smooth transformation for more beautiful result
    image = image.convertToFormat(QImage::Format_ARGB32).scaled(image.size()/1.8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    if (size)
        *size = QSizeF(w, h);
    return image;
}


QImage Renderer::renderToImage(const QUrl& url, Method method, QSizeF* size)
{
    switch(method)
    {
        case Method::PDF:
            return pdfRenderToImage(url, d->scale, d->useHighRes, size);

        case Method::EPS:
            return epsRenderToImage(url, d->scale, d->useHighRes, size);

        default:
            return QImage();
    }
}
