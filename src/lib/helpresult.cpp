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

#include <QJsonValue>

#include "helpresult.h"
using namespace Cantor;

class Cantor::HelpResultPrivate
{
public:
    HelpResultPrivate() = default;
    ~HelpResultPrivate() = default;

    QString html;
};

HelpResult::HelpResult(const QString& text, bool isHtml) : d(new HelpResultPrivate)
{
    QString html;
    if (!isHtml)
    {
        html = text.toHtmlEscaped();
        html.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
        html.replace(QLatin1Char('\n'), QLatin1String("<br/>\n"));
    }
    else
        html = text;

    d->html = html;
}

Cantor::HelpResult::~HelpResult()
{
    delete d;
}

int HelpResult::type()
{
    return HelpResult::Type;
}

QDomElement HelpResult::toXml(QDomDocument& doc)
{
    //No need to save results of a help request
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("help"));
    return e;
}

QJsonValue Cantor::HelpResult::toJupyterJson()
{
    // No need to save help result
    return QJsonValue();
}

QString HelpResult::toHtml()
{
    return d->html;
}

QVariant HelpResult::data()
{
    return QVariant(d->html);
}

QString HelpResult::mimeType()
{
    return QStringLiteral("text/html");
}

void HelpResult::save(const QString& filename)
{
    //No need to save results of a help request
    Q_UNUSED(filename);
}
