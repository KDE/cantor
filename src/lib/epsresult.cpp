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

#include <config-cantorlib.h>

#include <QDebug>
#include <QJsonValue>

#include <KZip>
#include <KIO/Job>
#include <QBuffer>
#include <QJsonObject>

#include "epsrenderer.h"
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
    e.setAttribute(QStringLiteral("type"), QStringLiteral("image"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());

#ifdef WITH_EPS
    const QImage& image = EpsRenderer::renderToImage(d->url, 1.0, false);
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

    const QImage& image = d->image.isNull() ? EpsRenderer::renderToImage(d->url, 1.0, false) : d->image;

    QJsonObject data;
    data.insert(JupyterUtils::pngMime, JupyterUtils::packMimeBundle(image, JupyterUtils::pngMime));
    root.insert(QLatin1String("data"), data);

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
