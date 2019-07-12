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

#include "textresult.h"
using namespace Cantor;

#include <QDebug>

#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>

QString rtrim(const QString& s)
{
    QString result = s;
    while (result.count() > 0 && result[result.count()-1].isSpace() )
    {
        result = result.left(result.count() -1 );
    }
    return result;
}

class Cantor::TextResultPrivate
{
public:
    TextResultPrivate()
    {
        format=TextResult::PlainTextFormat;
    }

    QString data;
    QString plain;
    TextResult::Format format;
};

TextResult::TextResult(const QString& data) : d(new TextResultPrivate)
{
    d->data=rtrim(data);
    d->plain=d->data;
}

TextResult::TextResult(const QString& data, const QString& plain) : d(new TextResultPrivate)
{
    d->data=rtrim(data);
    d->plain=rtrim(plain);
}


TextResult::~TextResult()
{
    delete d;
}

QString TextResult::toHtml()
{
    QString s=d->data.toHtmlEscaped();
    s.replace(QLatin1Char('\n'), QLatin1String("<br/>\n"));
    s.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
    return s;
}

QVariant TextResult::data()
{
    return QVariant(d->data);
}

QString TextResult::plain()
{
    return d->plain;
}

int TextResult::type()
{
    return TextResult::Type;
}

QString TextResult::mimeType()
{
    qDebug()<<"format: "<<format();
    switch(format())
    {
        case TextResult::PlainTextFormat:
            return QStringLiteral("text/plain");

        case TextResult::LatexFormat:
            return QStringLiteral("text/x-tex");

        default:
            return QString();
    }
}

TextResult::Format TextResult::format()
{
    return d->format;
}

void TextResult::setFormat(TextResult::Format f)
{
    d->format=f;
}

QDomElement TextResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving textresult "<<toHtml();
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("text"));
    if (d->format == LatexFormat)
        e.setAttribute(QStringLiteral("format"), QStringLiteral("latex"));
    QDomText txt=doc.createTextNode(data().toString());
    e.appendChild(txt);

    return e;
}

QJsonValue Cantor::TextResult::toJupyterJson()
{
    QJsonObject root;

    switch (d->format)
    {
        case PlainTextFormat:
        {
            root.insert(QLatin1String("output_type"), QLatin1String("stream"));
            root.insert(QLatin1String("name"), QLatin1String("stdout"));

            root.insert(QLatin1String("text"), jupyterText(d->data));
            break;
        }

        case LatexFormat:
        {
            root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

            QJsonObject data;
            data.insert(QLatin1String("text/latex"), jupyterText(d->data));
            data.insert(QLatin1String("text/plain"), jupyterText(d->plain));
            root.insert(QLatin1String("data"), data);

            root.insert(QLatin1String("metadata"), QJsonObject());
            break;
        }
    }

    return root;
}

void TextResult::save(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    stream<<d->data;

    file.close();
}

QJsonArray TextResult::jupyterText(const QString& text)
{
    QJsonArray array;

    const QStringList& lines = text.split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); i++)
    {
        QString line = lines[i];
        if (i != lines.size() - 1) // not last
            line.append(QLatin1Char('\n'));
        array.append(line);
    }

    return array;
}
