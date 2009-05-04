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

#include "helpresult.h"
using namespace MathematiK;

#include <QStringList>

HelpResult::HelpResult(const QString& text) : TextResult(text)
{

}

HelpResult::~HelpResult()
{

}

int HelpResult::type()
{
    return HelpResult::Type;
}

QDomElement HelpResult::toXml(QDomDocument& doc)
{
    QDomElement e=doc.createElement("Result");
    e.setAttribute("type", "help");
    //No need to save results of a help request
    QDomText txt=doc.createTextNode(QString::null);
    e.appendChild(txt);

    return e;
}

class MathematiK::ContextHelpResultPrivate
{
    public:
        QStringList entries;
};

ContextHelpResult::ContextHelpResult(const QStringList& entries) : Result(), d(new ContextHelpResultPrivate)
{
    d->entries=entries;
}

ContextHelpResult::~ContextHelpResult()
{
    delete d;
}

QString ContextHelpResult::toHtml()
{
    return "<justify>"+d->entries.join("\t ")+"</justify>";
}

QVariant ContextHelpResult::data()
{
    return d->entries;
}

int ContextHelpResult::type()
{
    return ContextHelpResult::Type;
}

QDomElement ContextHelpResult::toXml(QDomDocument& doc)
{
    Q_UNUSED(doc)
    //this will not be saved;
    return QDomElement();
}
