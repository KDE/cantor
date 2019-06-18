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

class Cantor::EpsResultPrivate{
    public:
        QUrl url;
};


EpsResult::EpsResult(const QUrl& url) : d(new EpsResultPrivate)
{
    d->url=url;
#ifndef WITH_EPS
    qDebug()<<"Creating an EpsResult in an environment compiled without EPS support!";
#endif
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
    qDebug()<<"done";

    return e;
}

QJsonValue Cantor::EpsResult::toJupyterJson()
{
    // Juputer TODO: Technically, I could convert .eps file to Image via EpsRenderer
    // But EpsRender don't available from Cantor core library
    // I will handle this result type in worksheet, but it's not look nice...

    return QJsonValue();
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
