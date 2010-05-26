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

#include <kdebug.h>

#include <QFile>
#include <QTextStream>
#include <QTextDocument>

class Cantor::TextResultPrivate
{
public:
    TextResultPrivate()
    {
        format=TextResult::PlainTextFormat;
    }

    QString data;
    TextResult::Format format;
};

TextResult::TextResult(const QString& data) : d(new TextResultPrivate)
{
    d->data=data;
}

TextResult::~TextResult()
{
    delete d;
}

QString TextResult::toHtml()
{
    return Qt::convertFromPlainText(d->data);
}

QVariant TextResult::data()
{
    return QVariant(d->data);
}

int TextResult::type()
{
    return TextResult::Type;
}

QString TextResult::mimeType()
{
    kDebug()<<"format: "<<format();
    if(format()==TextResult::PlainTextFormat)
        return "text/plain";
    else
        return "text/x-tex";
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
    kDebug()<<"saving textresult "<<toHtml();
    QDomElement e=doc.createElement("Result");
    e.setAttribute("type", "text");
    QDomText txt=doc.createTextNode(data().toString());
    e.appendChild(txt);

    return e;
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



