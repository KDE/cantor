/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "panelpluginhandler.h"
using namespace Cantor;

#include <QDebug>
#include <QDir>

#include <KPluginFactory>
#include <KPluginMetaData>
#include <KPluginFactory>

#include "session.h"
#include "backend.h"

class Cantor::PanelPluginHandlerPrivate
{
  public:
    QList<Cantor::PanelPlugin*> plugins;
};

PanelPluginHandler::PanelPluginHandler( QObject* parent ) : QObject(parent) ,
                                                            d(new PanelPluginHandlerPrivate)
{
    setObjectName(QStringLiteral("PanelPluginHandler"));
}

PanelPluginHandler::~PanelPluginHandler()
{
    delete d;
}

void PanelPluginHandler::loadPlugins()
{
    const QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("cantor_panels"));

    for (const KPluginMetaData &plugin : plugins) {

        const auto result = KPluginFactory::instantiatePlugin<PanelPlugin>(plugin, QCoreApplication::instance());

        if (!result) {
            qDebug() << "Error while loading panel: " << result.errorText;
            continue;
        }

        PanelPlugin *panel = result.plugin;

        panel->setPluginInfo(plugin);
        d->plugins.append(panel);

    }
}

QList<Cantor::PanelPlugin *> Cantor::PanelPluginHandler::allPlugins()
{
    return d->plugins;
}

QList<PanelPlugin*> PanelPluginHandler::plugins(Session* session)
{
    QList<PanelPlugin*> pluginsForSession;

    if (session == nullptr)
        return pluginsForSession;

    const auto capabilities = session->backend()->capabilities();
    const QStringList& extensions = session->backend()->extensions();

    qDebug()<<"loading panel plugins for session of type "<<session->backend()->name();
    for (auto* plugin : d->plugins)
    {
        bool supported=true;
        for (const QString& req : plugin->requiredExtensions()){
            // FIXME: That req.isEmpty() is there just because Help Panel has req
            // empty, returning FALSE when the comparison must to return TRUE.
            supported = supported && (extensions.contains(req) || req.isEmpty());
        }

        supported = supported && ( (capabilities & plugin->requiredCapabilities()) == plugin->requiredCapabilities());

        if(supported)
        {
            qDebug() << "plugin " << plugin->name()<<" is supported, requires extensions " << plugin->requiredExtensions();
            pluginsForSession.append(plugin);
        }
        else
            qDebug() << "plugin " << plugin->name() <<" is not supported";
    }

    return pluginsForSession;
}

QList<PanelPlugin*> PanelPluginHandler::activePluginsForSession(Session* session, const PanelStates& previousPluginStates)
{
    const auto& plugins = this->plugins(session);
    for (auto* plugin : plugins)
    {
        if(!plugin)
        {
            qDebug()<<"somethings wrong with plugin inside PanelPluginHandler";
            continue;
        }

        if (previousPluginStates.contains(plugin->name()))
            plugin->restoreState(previousPluginStates[plugin->name()]);
        else
        {
            Cantor::PanelPlugin::State initState;
            initState.session = session;
            plugin->restoreState(initState);
        }
    }
    return plugins;
}
