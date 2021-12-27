/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "panelplugin.h"
using namespace Cantor;

#include <KPluginMetaData>

class Cantor::PanelPluginPrivate
{
  public:
    QString name;
    QStringList requiredExtensions;
    Session* session = nullptr;
    QWidget* parentWidget = nullptr;
};

PanelPlugin::PanelPlugin( QObject* parent) : QObject(parent), /* KXMLGUIClient(dynamic_cast<KXMLGUIClient*>(parent)),*/
                                             d(new PanelPluginPrivate)
{

}

PanelPlugin::~PanelPlugin()
{
    delete d;
}

void PanelPlugin::setParentWidget(QWidget* widget)
{
    d->parentWidget = widget;
}

QWidget* PanelPlugin::parentWidget()
{
    return d->parentWidget;
}

void PanelPlugin::setPluginInfo(const KPluginMetaData& info)
{
    d->name = info.name();
    d->requiredExtensions = info.value(QStringLiteral("RequiredExtensions")).split(QLatin1Char(','));
    setObjectName(info.pluginId());
}

QStringList PanelPlugin::requiredExtensions()
{
    return d->requiredExtensions;
}

Backend::Capabilities PanelPlugin::requiredCapabilities()
{
    return Backend::Nothing;
}

QString PanelPlugin::name()
{
    return d->name;
}

Cantor::PanelPlugin::State Cantor::PanelPlugin::saveState()
{
    Cantor::PanelPlugin::State state;
    state.session = d->session;
    return state;
}

void Cantor::PanelPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    d->session = state.session;
}

Cantor::Session* Cantor::PanelPlugin::session()
{
    return d->session;
}

void Cantor::PanelPlugin::connectToShell(QObject* /* cantorShell */)
{

}

bool Cantor::PanelPlugin::showOnStartup()
{
    return true;
}
