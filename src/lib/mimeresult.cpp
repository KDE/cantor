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
    Copyright (C) 2019 Nikita Sirgienko <warquark@gmail.com>
*/

#include "mimeresult.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <KLocalizedString>

using namespace Cantor;

class Cantor::MimeResultPrivate
{
public:
    MimeResultPrivate() = default;

    QString plain;
    QJsonValue content;
    QString mimeType;
    bool isOriginalPlain;
};

MimeResult::MimeResult(const QString& plain, const QJsonValue& content, const QString mimeType) : d(new MimeResultPrivate)
{
    d->isOriginalPlain = !plain.isEmpty();
    if (d->isOriginalPlain)
        d->plain = plain;
    else
        d->plain = i18n("This is unsupported Jupyter content of type '%1'", mimeType);
    d->content = content;
    d->mimeType = mimeType;
}

MimeResult::~MimeResult()
{
    delete d;
}

QString MimeResult::toHtml()
{
    return QLatin1String("<pre>") + d->plain.toHtmlEscaped() + QLatin1String("</pre>");
}

int MimeResult::type()
{
    return MimeResult::Type;
}

QString MimeResult::mimeType()
{
    return QLatin1Literal("application/json");
}

QVariant MimeResult::data()
{
    return d->content;
}

QString MimeResult::plain()
{
    return d->plain;
}

QString MimeResult::mimeKey()
{
    return d->mimeType;
}

QDomElement MimeResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving mime result with type" << d->mimeType;
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("mime"));
    e.setAttribute(QStringLiteral("mimeType"), d->mimeType);
    e.setAttribute(QStringLiteral("withPlain"), d->isOriginalPlain);
    if (d->isOriginalPlain)
    {
        QDomElement plain = doc.createElement(QStringLiteral("Plain"));
        plain.appendChild(doc.createTextNode(d->plain));
        e.appendChild(plain);
    }

    QJsonDocument jsonDoc;
    QJsonObject obj;
    obj.insert(QLatin1String("content"), d->content);
    jsonDoc.setObject(obj);

    QDomElement content = doc.createElement(QStringLiteral("Content"));
    content.appendChild(doc.createTextNode(QString::fromUtf8(jsonDoc.toJson())));
    e.appendChild(content);

    return e;
}

QJsonValue Cantor::MimeResult::toJupyterJson()
{
    QJsonObject root;
    root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    QJsonObject data;
    data.insert(d->mimeType, d->content);

    QJsonArray array;
    const QStringList& lines = d->plain.split(QLatin1Char('\n'));
    for (QString line : lines)
    {
        line.append(QLatin1Char('\n'));
        array.append(line);
    }
    if (d->isOriginalPlain)
        data.insert(QLatin1String("text/plain"), array);

    root.insert(QLatin1String("data"), data);
    root.insert(QLatin1String("metadata"), QJsonObject());

    return root;
}

void Cantor::MimeResult::save(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    QJsonObject root;
    root.insert(d->mimeType, d->content);

    QJsonArray array;
    const QStringList& lines = d->plain.split(QLatin1Char('\n'));
    for (QString line : lines)
    {
        line.append(QLatin1Char('\n'));
        array.append(line);
    }
    if (d->isOriginalPlain)
        root.insert(QLatin1String("text/plain"), array);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(root);

    stream << jsonDoc.toJson();

    file.close();
}
