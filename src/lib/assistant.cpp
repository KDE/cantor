/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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

void Assistant::setPluginInfo(const KPluginMetaData &info)
{
    d->name=info.name();
    d->icon=info.iconName();
    d->requiredExtensions=info.value(QStringLiteral("RequiredExtensions")).split(QLatin1Char(','));
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
