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
    Copyright (C) 2019 Sirgienko Nikita <warquark@gmail.com>
 */

#include "htmlresult.h"

#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "jupyterutils.h"

using namespace Cantor;

class Cantor::HtmlResultPrivate
{
public:
    QString html;
    QString plain;
    std::map<QString, QJsonValue> alternatives; // Usefull only for Jupyter, it think
    Cantor::HtmlResult::Format format{Cantor::HtmlResult::Html};
};

HtmlResult::HtmlResult(const QString& html, const QString& plain, const std::map<QString, QJsonValue>& alternatives) : d(new HtmlResultPrivate())
{
    d->html = html;
    d->plain = plain;
    d->alternatives = alternatives;
}

HtmlResult::~HtmlResult()
{
    delete d;
}

QString HtmlResult::toHtml()
{
    switch(d->format)
    {
        case HtmlResult::Html:
            return d->html;

        case HtmlResult::HtmlSource:
            return QStringLiteral("<code><pre>") + d->html.toHtmlEscaped() + QStringLiteral("</pre></code>");

        case HtmlResult::PlainAlternative:
            return QStringLiteral("<pre>") + d->plain.toHtmlEscaped() + QStringLiteral("</pre>");

        default:
            return QString();
    }
}

QVariant Cantor::HtmlResult::data()
{
    return d->html;
}

QString Cantor::HtmlResult::plain()
{
    return d->plain;
}

void Cantor::HtmlResult::setFormat(HtmlResult::Format format)
{
    d->format = format;
}

HtmlResult::Format Cantor::HtmlResult::format()
{
    return d->format;
}

int Cantor::HtmlResult::type()
{
    return HtmlResult::Type;
}

QString Cantor::HtmlResult::mimeType()
{
    return QStringLiteral("text/html");
}

QDomElement Cantor::HtmlResult::toXml(QDomDocument& doc)
{
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("html"));
    switch(d->format)
    {
        case HtmlResult::HtmlSource:
            e.setAttribute(QStringLiteral("format"), QStringLiteral("htmlSource"));

        case HtmlResult::PlainAlternative:
            e.setAttribute(QStringLiteral("format"), QStringLiteral("plain"));

        // Html format used by default, so don't set it
        default:
            break;
    }

    QDomElement plainE = doc.createElement(QStringLiteral("Plain"));
    plainE.appendChild(doc.createTextNode(d->plain));
    e.appendChild(plainE);

    QDomElement htmlE = doc.createElement(QStringLiteral("Html"));
    htmlE.appendChild(doc.createTextNode(d->html));
    e.appendChild(htmlE);

    for (auto iter = d->alternatives.begin(); iter != d->alternatives.end(); iter++)
    {
        QJsonDocument jsonDoc;
        QJsonObject obj;
        obj.insert(QLatin1String("root"), iter->second);
        jsonDoc.setObject(obj);

        QDomElement content = doc.createElement(QStringLiteral("Alternative"));
        content.setAttribute(QStringLiteral("key"), iter->first);
        content.appendChild(doc.createTextNode(QString::fromUtf8(jsonDoc.toJson())));
        e.appendChild(content);
    }

    return e;
}

QJsonValue Cantor::HtmlResult::toJupyterJson()
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
    data.insert(QLatin1String("text/html"), JupyterUtils::toJupyterMultiline(d->html));
    if (!d->plain.isEmpty())
        data.insert(QLatin1String("text/plain"), JupyterUtils::toJupyterMultiline(d->plain));

    for (auto iter = d->alternatives.begin(); iter != d->alternatives.end(); iter++)
        data.insert(iter->first, iter->second);

    root.insert(QLatin1String("data"), data);

    root.insert(QLatin1String("metadata"), jupyterMetadata());

    return root;
}

void Cantor::HtmlResult::save(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    stream<<d->html;

    file.close();
}
