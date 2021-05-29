/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "animationresult.h"
using namespace Cantor;

#include <QImage>
#include <QImageWriter>
#include <KZip>
#include <QMimeDatabase>
#include <QDebug>
#include <KIO/Job>
#include <QMovie>

class Cantor::AnimationResultPrivate
{
  public:
    AnimationResultPrivate() = default;

    QUrl url;
    QMovie* movie;
    QString alt;
};

AnimationResult::AnimationResult(const QUrl &url, const QString& alt ) : d(new AnimationResultPrivate)
{
    d->url=url;
    d->alt=alt;
    d->movie=new QMovie();
    d->movie->setFileName(url.toLocalFile());
}


AnimationResult::~AnimationResult()
{
    delete d->movie;
    delete d;
}

QString AnimationResult::toHtml()
{
    return QStringLiteral("<img src=\"%1\" alt=\"%2\"/>").arg(d->url.toLocalFile(), d->alt);
}

QVariant AnimationResult::data()
{
    return QVariant::fromValue(static_cast<QObject*>(d->movie));
}

QUrl AnimationResult::url()
{
    return d->url;
}

int AnimationResult::type()
{
    return AnimationResult::Type;
}

QString AnimationResult::mimeType()
{
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForUrl(d->url);

    return type.name();
}

QDomElement AnimationResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving imageresult "<<toHtml();
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("animation"));
    e.setAttribute(QStringLiteral("filename"), d->url.fileName());
    qDebug()<<"done";

    return e;
}

QJsonValue Cantor::AnimationResult::toJupyterJson()
{
    QJsonObject root;

    if (executionIndex() != -1)
    {
        root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
        root.insert(QLatin1String("execution_count"), executionIndex());
    }
    else
        root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    QJsonObject data;
    data.insert(QLatin1String("text/plain"), d->alt);

    QFile file(d->url.toLocalFile());
    QByteArray bytes;
    if (file.open(QIODevice::ReadOnly))
        bytes = file.readAll();
    data.insert(QLatin1String("image/gif"), QString::fromLatin1(bytes.toBase64()));

    root.insert(QLatin1String("data"), data);
    // Not sure, but in Jupyter size of gif doesn't controlled by metadata unlike ImageResult
    root.insert(QLatin1String("metadata"), jupyterMetadata());

    return root;
}

void AnimationResult::saveAdditionalData(KZip* archive)
{
    archive->addLocalFile(d->url.toLocalFile(), d->url.fileName());
}

void AnimationResult::save(const QString& filename)
{
    //just copy over the file..
    KIO::file_copy(d->url, QUrl::fromLocalFile(filename), -1, KIO::HideProgressInfo);
}
