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
    Copyright (C) 2010 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "panelplugin.h"
using namespace Cantor;

#include <KPluginMetaData>

class Cantor::PanelPluginPrivate
{
  public:
    QString name;
    QStringList requiredExtensions;
    Session* session;
    QWidget* parentWidget;
};

PanelPlugin::PanelPlugin( QObject* parent) : QObject(parent), /* KXMLGUIClient(dynamic_cast<KXMLGUIClient*>(parent)),*/
                                             d(new PanelPluginPrivate)
{
    d->parentWidget=nullptr;
    d->session=nullptr;
}

PanelPlugin::~PanelPlugin()
{
    delete d;
}

void PanelPlugin::setParentWidget(QWidget* widget)
{
    d->parentWidget=widget;
}

QWidget* PanelPlugin::parentWidget()
{
    return d->parentWidget;
}

void PanelPlugin::setPluginInfo(const KPluginMetaData& info)
{
    d->name=info.name();
    d->requiredExtensions=info.value(QStringLiteral("RequiredExtensions")).split(QLatin1Char(','));
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

Cantor::Session * Cantor::PanelPlugin::session()
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
