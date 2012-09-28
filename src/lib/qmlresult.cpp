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
    Copyright (C) 2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "qmlresult.h"
using namespace Cantor;

#include "kurl.h"
#include <QImage>
#include <QImageWriter>
#include <kdebug.h>
#include <ktemporaryfile.h>
#include <kzip.h>

#include <QDeclarativeItem>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

class Cantor::QmlResultPrivate
{
    public:
        QString qml;
        ContextPropertyList properties;

        KUrl tmpFile;
};

QmlResult::QmlResult(const QString& qml, const ContextPropertyList& properties)
{
    d->qml=qml;
    d->properties=properties;
}

QmlResult::~QmlResult()
{

}

QString QmlResult::toHtml()
{
    return d->qml;
}

QVariant QmlResult::data()
{
    return d->qml;
}

QString QmlResult::plain()
{
    return d->qml;
}

int QmlResult::type()
{
    return Type;
}

QString QmlResult::mimeType()
{
    const QList<QByteArray> formats=QImageWriter::supportedImageFormats();
    QString mimetype;
    foreach(const QByteArray& format, formats)
    {
        mimetype+="image/"+format.toLower()+' ';
    }
    kDebug()<<"type: "<<mimetype;

    return mimetype;
}

QDomElement QmlResult::toXml(QDomDocument& doc)
{
    kDebug()<<"saving mml result "<<toHtml();
    KTemporaryFile tmp;
    tmp.setSuffix(".png");

    d->tmpFile=KUrl(tmp.fileName());
    QDomElement e=doc.createElement("Result");
    e.setAttribute("type", "image");
    e.setAttribute("filename", d->tmpFile.fileName());
    kDebug()<<"done";

    return e;
}

void QmlResult::save(const QString& filename)
{
    QImage img=renderToImage();

    img.save(filename);
}

void QmlResult::saveAdditionalData(KZip* archive)
{
    save(d->tmpFile.toLocalFile());
    archive->addLocalFile(d->tmpFile.toLocalFile(), d->tmpFile.fileName());
}

QImage QmlResult::renderToImage()
{
    QDeclarativeEngine engine;
    QDeclarativeContext *context = new QDeclarativeContext(engine.rootContext());
    foreach(const ContextProperty& property, d->properties)
    {
        context->setContextProperty(property.name, property.value);
    }

    QDeclarativeComponent component(&engine);
    component.setData(d->qml.toUtf8(), QUrl());

    QObject *myObject = component.create();
    QDeclarativeItem *item = qobject_cast<QDeclarativeItem*>(myObject);
    QImage pix(item->width(),  item->height(), QImage::Format_ARGB32);
    QPainter painter(&pix);
    QStyleOptionGraphicsItem option;
    item->paint(&painter,  &option,  NULL);

    return pix;
}
