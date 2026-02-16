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
    QTextImageFormat format = render(document, Method::PDF, QUrl::fromLocalFile(latex->imagePath()), latex->uuid());

    if (!format.name().isEmpty())
    {
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
    if (pdfPage == nullptr)
    {
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library failed to access first page of %1 document").arg(url.toLocalFile());

        return QImage();
    }

    QSize pageSize = pdfPage->pageSize();

    double realScale = 2.0 * 1.8;
    qreal w = 2.0 * pageSize.width();
    qreal h = 2.0 * pageSize.height();
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
    return pdfRenderToImage(url, d->scale, d->useHighRes, size);
}
