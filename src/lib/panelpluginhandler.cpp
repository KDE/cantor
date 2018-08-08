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

#include "panelpluginhandler.h"
using namespace Cantor;

#include <QDebug>
#include <QDir>
#include <KService>
#include <KServiceTypeTrader>
#include <KPluginMetaData>

#include "session.h"
#include "panelplugin.h"
#include "backend.h"

class Cantor::PanelPluginHandlerPrivate
{
  public:
    QList<Cantor::PanelPlugin*> plugins;
    Cantor::Session* session;
};

PanelPluginHandler::PanelPluginHandler( QObject* parent ) : QObject(parent) ,
                                                            d(new PanelPluginHandlerPrivate)
{
    setObjectName(QLatin1String("PanelPluginHandler"));
    d->session=nullptr;
}

PanelPluginHandler::~PanelPluginHandler()
{
    delete d;
}

void PanelPluginHandler::loadPlugins()
{
    if(d->session==nullptr)
        return;
    qDebug()<<"loading panel plugins for session of type "<<d->session->backend()->name();

    QStringList panelDirs;
    foreach(const QString &dir, QCoreApplication::libraryPaths()){
        panelDirs << dir + QDir::separator() + QLatin1String("cantor/panels");
    }

    QPluginLoader loader;
    const Cantor::Backend::Capabilities capabilities = d->session->backend()->capabilities();
    const QStringList& extensions = d->session->backend()->extensions();
    foreach(const QString &dir, panelDirs){

        qDebug() << "dir: " << dir;
        QStringList panels;
        QDir panelDir = QDir(dir);

        panels = panelDir.entryList();

        foreach (const QString &panel, panels){
            if (panel==QLatin1String(".") || panel==QLatin1String(".."))
                continue;

            loader.setFileName(dir + QDir::separator() + panel);

            if (!loader.load()){
                qDebug() << "Error while loading panel: " << panel;
                continue;
            }

            KPluginFactory* factory = KPluginLoader(loader.fileName()).factory();
            PanelPlugin* plugin = factory->create<PanelPlugin>(this);

            KPluginMetaData info(loader);
            plugin->setPluginInfo(info);

            bool supported=true;
            foreach(const QString& req, plugin->requiredExtensions()){
                // FIXME: That req.isEmpty() is there just because Help Panel has req
                // empty, returning FALSE when the comparision must to return TRUE.
                supported = supported && (extensions.contains(req) || req.isEmpty());
            }

            supported = supported && ( (capabilities & plugin->requiredCapabilities()) == plugin->requiredCapabilities());

            if(supported)
            {
                qDebug() << "plugin " << info.name()<<" is supported, requires extensions " << plugin->requiredExtensions();
                d->plugins.append(plugin);
                plugin->setSession(d->session);
            }else
            {
                qDebug() << "plugin " << info.name() <<" is not supported";
                plugin->deleteLater();
            }
        }
    }

    emit pluginsChanged();
}

void PanelPluginHandler::setSession(Session* session)
{
    qDeleteAll(d->plugins);
    d->plugins.clear();
    d->session=session;
    loadPlugins();
}

QList<PanelPlugin*> PanelPluginHandler::plugins()
{
    return d->plugins;
}

void PanelPluginHandler::addPlugin(PanelPlugin* plugin)
{
    d->plugins.append(plugin);
}


