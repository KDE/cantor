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

#include "assistant.h"
using namespace Cantor;

class Cantor::AssistantPrivate
{
  public:
    QString name;
    QString icon;
    QStringList requiredExtensions;
    Backend* backend;
};

Assistant::Assistant(QObject* parent) : QObject(parent), KXMLGUIClient(dynamic_cast<KXMLGUIClient*>(parent)),
                                        d(new AssistantPrivate)
{

}

Assistant::~Assistant()
{
    delete d;
}

void Assistant::setBackend(Cantor::Backend* backend)
{
    d->backend=backend;
}

void Assistant::setPluginInfo(KPluginMetaData info)
{
    d->name=info.name();
    d->icon=info.iconName();
    d->requiredExtensions=info.value(QLatin1String("RequiredExtensions")).split(QLatin1Char(','));
}


QStringList Assistant::requiredExtensions()
{
    return d->requiredExtensions;
}

QString Assistant::icon()
{
    return d->icon;
}

QString Assistant::name()
{
    return d->name;
}

Backend* Assistant::backend()
{
    return d->backend;
}
