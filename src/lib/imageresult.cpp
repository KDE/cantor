/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "imageresult.h"
using namespace Cantor;

#include <QImage>
#include <QImageWriter>
#include <KZip>
#include <QDebug>
#include <QBuffer>
#include <QTemporaryFile>

#include "jupyterutils.h"

class Cantor::ImageResultPrivate
{
  public:
    ImageResultPrivate() = default;

    QUrl url;
    QImage img;
    QString alt;
    QSize displaySize;

    QString originalFormat{JupyterUtils::pngMime};
    QString svgContent; // HACK: qt can't easily render svg, so, if we load the result from Jupyter svg image, store original svg
};

ImageResult::ImageResult(const QUrl &url, const QString& alt) :  d(new ImageResultPrivate)
{
    d->url=url;
    d->alt=alt;
}

Cantor::ImageResult::ImageResult(const QImage& image, const QString& alt) :  d(new ImageResultPrivate)
{
    d->img=image;
    d->alt=alt;

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
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("image"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());
    if (!d->alt.isEmpty())
        e.appendChild(doc.createTextNode(d->alt));
    qDebug()<<"done";

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
        data = JupyterUtils::packMimeBundle(image, d->originalFormat);

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

void ImageResult::save(const QString& filename)
{
    bool rc = d->img.save(filename);
    if (!rc)
        qDebug()<<"saving to " << filename << " failed.";
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
