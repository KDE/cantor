/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "renderer.h"

#include <QUuid>
#include <QDebug>
#include <QMutex>
#include <QScreen>

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

QTextImageFormat Renderer::render(QTextDocument *document, const QUrl &url, const QString& uuid)
{
    QTextImageFormat format;

    QUrl internal;
    internal.setScheme(QLatin1String("internal"));
    internal.setPath(uuid);

    QSizeF s = renderToResource(document,url, internal);

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
    QTextImageFormat format = render(document, QUrl::fromLocalFile(latex->imagePath()), latex->uuid());

    if (!format.name().isEmpty())
    {
        format.setProperty(CantorFormula, latex->method());
        format.setProperty(ImagePath, latex->imagePath());
        format.setProperty(Code, latex->latexCode());
    }

    return format;
}

QSizeF Renderer::renderToResource(QTextDocument *document,const QUrl &url, const QUrl& internal)
{
    QSizeF size;
    QImage img = renderToImage(url, &size);

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

    document->setRenderHint(Poppler::Document::Antialiasing, true);
    document->setRenderHint(Poppler::Document::TextAntialiasing, true);
    document->setRenderHint(Poppler::Document::TextHinting, true);

    auto pdfPage = document->page(0);
    if (pdfPage == nullptr)
    {
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library failed to access first page of %1 document").arg(url.toLocalFile());
        return QImage();
    }

    double dpiX = QGuiApplication::primaryScreen()->physicalDotsPerInchX();

    double superSample = 2.0;
    double effectiveScale = (2.0 / 1.8) * scale * superSample;

    if (highResolution)
        effectiveScale = (2.0 / 1.8) * 5.0 * superSample;

    double targetDpi = dpiX * effectiveScale;
    QImage image = pdfPage->renderToImage(targetDpi, targetDpi);

    popplerMutex.lock();
    popplerMutex.unlock();

    if (image.isNull())
    {
        if (errorReason)
            *errorReason = QString::fromLatin1("Poppler library failed to render pdf %1 to image").arg(url.toLocalFile());
        return image;
    }

    if (image.format() != QImage::Format_ARGB32_Premultiplied)
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    if (size)
        *size = QSizeF(image.width() / superSample, image.height() / superSample);

    return image;
}
QImage Renderer::renderToImage(const QUrl& url, QSizeF* size)
{
    return pdfRenderToImage(url, d->scale, d->useHighRes, size);
}
