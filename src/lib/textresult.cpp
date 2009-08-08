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
using namespace MathematiK;

#include <kdebug.h>

class MathematiK::TextResultPrivate
{
public:
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
    QString s=d->data;
    s.replace('\n', "<br/>\n");
    return s;
}

QVariant TextResult::data()
{
    return QVariant(d->data);
}

int TextResult::type()
{
    return TextResult::Type;
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



