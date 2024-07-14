/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2022 by Alexander Semke (alexander.semke@web.de)
*/

#include "imageresult.h"
#include "jupyterutils.h"
using namespace Cantor;

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QImageWriter>
#include <QPainter>
#include <QScreen>
#include <QSvgRenderer>
#include <QTemporaryFile>

#include <KZip>

#include <poppler-qt6.h>

class Cantor::ImageResultPrivate
{
  public:
    ImageResultPrivate() = default;

    QUrl url;
    QImage img;
    QString alt;
    QSize displaySize;
    QString extension;
    QByteArray data; // byte array used to store the contnent of PDF and SVG files

    QString originalFormat{JupyterUtils::pngMime};
    QString svgContent; // HACK: qt can't easily render svg, so, if we load the result from Jupyter svg image, store original svg
};

ImageResult::ImageResult(const QUrl &url, const QString& alt) :  d(new ImageResultPrivate)
{
    d->url = url;
    d->alt = alt;
    d->extension = url.toLocalFile().right(3).toLower();

    if (d->extension == QLatin1String("pdf") || d->extension == QLatin1String("svg")) // vector formats
    {
        QFile file(url.toLocalFile());
        if (!file.open(QIODevice::ReadOnly))
            return;

        d->data = file.readAll();
        if (d->data.isEmpty())
            return;

        const int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInchX();

        if (d->extension == QLatin1String("pdf"))
        {
            auto document = Poppler::Document::loadFromData(d->data);
            if (!document) {
                qDebug()<< "Failed to process the byte array of the PDF file " << url.toLocalFile();
                return;
            }

            auto page = document->page(0);
            if (!page) {
                qDebug() << "Failed to process the first page in the PDF file.";
                return;
            }

            document->setRenderHint(Poppler::Document::TextAntialiasing);
            document->setRenderHint(Poppler::Document::Antialiasing);
            document->setRenderHint(Poppler::Document::TextHinting);
            document->setRenderHint(Poppler::Document::TextSlightHinting);
            document->setRenderHint(Poppler::Document::ThinLineSolid);

            d->img = page->renderToImage(dpi, dpi);
        }
        else
        {
            QSvgRenderer renderer(d->data);

            // SVG document size is in points, convert to pixels
            const auto& size = renderer.defaultSize();
            int w = size.width() / 72 * dpi;
            int h = size.height() / 72 * dpi;
            d->img = QImage(w, h, QImage::Format_ARGB32);

            // render
            QPainter painter;
            painter.begin(&d->img);
            renderer.render(&painter);
            painter.end();
        }
    }
    else // raster formats
        d->img.load(d->url.toLocalFile());
}

Cantor::ImageResult::ImageResult(const QImage& image, const QString& alt) :  d(new ImageResultPrivate)
{
    d->img = image;
    d->alt = alt;

    QTemporaryFile imageFile;
    imageFile.setAutoRemove(false);
    if (imageFile.open())
    {
        d->img.save(imageFile.fileName(), "PNG");
        d->url = QUrl::fromLocalFile(imageFile.fileName());
    }
}

ImageResult::~ImageResult()
{
    delete d;
}

QString ImageResult::toHtml()
{
    return QStringLiteral("<img src=\"%1\" alt=\"%2\"/>").arg(d->url.toLocalFile(), d->alt);
}

QString ImageResult::toLatex()
{
    return QStringLiteral(" \\begin{center} \n \\includegraphics[width=12cm]{%1} \n \\end{center}").arg(d->url.fileName());
}

QVariant ImageResult::data()
{
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
    QString mimetype;
    for (const auto& format : QImageWriter::supportedImageFormats())
        mimetype += QLatin1String("image/" + format.toLower() + ' ');

    return mimetype;
}

QString ImageResult::extension()
{
    return d->extension;
}

QDomElement ImageResult::toXml(QDomDocument& doc)
{
    auto e = doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("image"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());

    if (!d->alt.isEmpty())
        e.appendChild(doc.createTextNode(d->alt));

    return e;
}

QJsonValue Cantor::ImageResult::toJupyterJson()
{
    QJsonObject root;

    if (executionIndex() != -1)
    {
        root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
        root.insert(QLatin1String("execution_count"), executionIndex());
    }
    else
        root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    QImage image;
    if (d->img.isNull())
        image.load(d->url.toLocalFile());
    else
        image = d->img;

    QJsonObject data;

    // HACK: see ImageResultPrivate::svgContent
    if (d->originalFormat == JupyterUtils::svgMime)
        data.insert(JupyterUtils::svgMime, JupyterUtils::toJupyterMultiline(d->svgContent));
    else
        data = JupyterUtils::packMimeBundle(d->img, d->originalFormat);

    data.insert(JupyterUtils::textMime, JupyterUtils::toJupyterMultiline(d->alt));


    root.insert(QLatin1String("data"), data);

    QJsonObject metadata(jupyterMetadata());
    if (d->displaySize.isValid())
    {
        QJsonObject size;
        size.insert(QLatin1String("width"), displaySize().width());
        size.insert(QLatin1String("height"), displaySize().height());
        metadata.insert(d->originalFormat, size);
    }
    root.insert(QLatin1String("metadata"), metadata);

    return root;
}

void ImageResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

void ImageResult::save(const QString& fileName)
{
    bool rc = false;
    if (d->extension == QLatin1String("pdf") || d->extension == QLatin1String("svg"))
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            if (file.write(d->data) > 0)
                rc = true;
            file.close();
        }
    }
    else
        rc = d->img.save(fileName);

    if (!rc)
        qDebug()<<"saving to " << fileName << " failed.";
}

QSize Cantor::ImageResult::displaySize()
{
    return d->displaySize;
}

void Cantor::ImageResult::setDisplaySize(QSize size)
{
    d->displaySize = size;
}

void Cantor::ImageResult::setOriginalFormat(const QString& format)
{
    d->originalFormat = format;
}

QString Cantor::ImageResult::originalFormat()
{
    return d->originalFormat;
}

void Cantor::ImageResult::setSvgContent(const QString& svgContent)
{
    d->svgContent = svgContent;
}
