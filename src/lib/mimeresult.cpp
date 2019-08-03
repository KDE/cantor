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

#include "jupyterutils.h"

using namespace Cantor;

class Cantor::MimeResultPrivate
{
public:
    MimeResultPrivate() = default;

    QString plain;
    QJsonObject mimeBundle;
};

MimeResult::MimeResult(const QJsonObject& mimeBundle) : d(new MimeResultPrivate)
{
    bool isOriginalPlain = mimeBundle.contains(QLatin1String("text/plain"));
    if (isOriginalPlain)
        d->plain = JupyterUtils::fromJupyterMultiline(mimeBundle.value(QLatin1String("text/plain")));
    else
        d->plain = i18n("This is unsupported Jupyter content of types ('%1')", mimeBundle.keys().join(QLatin1String(", ")));
    d->mimeBundle = mimeBundle;
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
    return d->mimeBundle;
}

QString MimeResult::plain()
{
    return d->plain;
}

QDomElement MimeResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving mime result with types" << d->mimeBundle.keys();
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("mime"));

    for (const QString& key : d->mimeBundle.keys())
    {
        QJsonDocument jsonDoc;
        QJsonObject obj;
        obj.insert(QLatin1String("content"), d->mimeBundle[key]);
        jsonDoc.setObject(obj);

        QDomElement content = doc.createElement(QStringLiteral("Content"));
        content.setAttribute(QStringLiteral("key"), key);
        content.appendChild(doc.createTextNode(QString::fromUtf8(jsonDoc.toJson())));
        e.appendChild(content);
    }

    return e;
}

QJsonValue Cantor::MimeResult::toJupyterJson()
{
    QJsonObject root;
    if (executionIndex() != -1)
    {
        root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
        root.insert(QLatin1String("execution_count"), executionIndex());
    }
    else
        root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    root.insert(QLatin1String("data"), d->mimeBundle);
    root.insert(QLatin1String("metadata"), jupyterMetadata());

    return root;
}

void Cantor::MimeResult::save(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(d->mimeBundle);

    stream << jsonDoc.toJson();

    file.close();
}
