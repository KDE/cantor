/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "epsresult.h"
using namespace Cantor;

#include <config-cantorlib.h>

#include <QDebug>
#include <QJsonValue>

#include <KZip>
#include <KIO/Job>
#include <QBuffer>
#include <QJsonObject>

#include "renderer.h"
#include "jupyterutils.h"

class Cantor::EpsResultPrivate{
    public:
        QUrl url;
        QImage image;
};


EpsResult::EpsResult(const QUrl& url, const QImage& image) : d(new EpsResultPrivate)
{
    d->url=url;
    d->image = image;
}

EpsResult::~EpsResult()
{
    delete d;
}

QString EpsResult::toHtml()
{
    return QStringLiteral("<img src=\"%1\" />").arg(d->url.url());
}

QString EpsResult::toLatex()
{
    return QStringLiteral(" \\begin{center} \n \\includegraphics[width=12cm]{%1}\n \\end{center}").arg(d->url.fileName());
}

QVariant EpsResult::data()
{
    return QVariant(d->url);
}

QUrl EpsResult::url()
{
    return d->url;
}

QImage Cantor::EpsResult::image()
{
    return d->image;
}

int EpsResult::type()
{
    return EpsResult::Type;
}

QString EpsResult::mimeType()
{
    return QStringLiteral("image/x-eps");
}

QDomElement EpsResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving imageresult "<<toHtml();
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("epsimage"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());

#ifdef WITH_EPS
    const QImage& image = Renderer::epsRenderToImage(d->url, 1.0, false);
    qDebug() << image.size() << image.isNull();
    if (!image.isNull())
    {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        e.setAttribute(QLatin1String("image"), QString::fromLatin1(ba.toBase64()));
    }
#else
    if (!d->image.isNull())
    {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        d->image.save(&buffer, "PNG");
        e.setAttribute(QLatin1String("image"), QString::fromLatin1(ba.toBase64()));
    }
#endif

    qDebug()<<"done";
    return e;
}

QJsonValue Cantor::EpsResult::toJupyterJson()
{
    QJsonObject root;

    if (executionIndex() != -1)
    {
        root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
        root.insert(QLatin1String("execution_count"), executionIndex());
    }
    else
        root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    const QImage& image = d->image.isNull() ? Renderer::epsRenderToImage(d->url, 1.0, false) : d->image;

    QJsonObject data;
    data = JupyterUtils::packMimeBundle(image, JupyterUtils::pngMime);
    root.insert(QLatin1String("data"), data);

    root.insert(QLatin1String("metadata"), jupyterMetadata());

    return root;
}

void EpsResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

void EpsResult::save(const QString& filename)
{
    //just copy over the eps file..
    KIO::file_copy(d->url, QUrl::fromLocalFile(filename), -1, KIO::HideProgressInfo);
}
