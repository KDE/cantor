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

#include "backend.h"
using namespace Cantor;

#include <KServiceTypeTrader>
#include <KService>
#include <QDebug>
#include <KXMLGUIFactory>
#include <KPluginInfo>

#include "extension.h"

class Cantor::BackendPrivate
{
  public:
    QString name;
    QString comment;
    QString icon;
    QString url;
    bool enabled;
};

Backend::Backend(QObject* parent, const QList<QVariant> args) : QObject(parent),
                                                                d(new BackendPrivate)
{
    Q_UNUSED(args)
    d->enabled=true;
}

Backend::~Backend()
{
    delete d;
}

QString Backend::name() const
{
    return d->name;
}

QString Backend::comment() const
{
    return d->comment;
}

QString Backend::description() const
{
    return comment();
}

QString Backend::icon() const
{
    return d->icon;
}

QString Backend::url() const
{
    return d->url;
}

KUrl Backend::helpUrl() const
{
    return KUrl();
}

bool Backend::isEnabled() const
{
    return d->enabled&&requirementsFullfilled();
}

void Backend::setEnabled(bool enabled)
{
    d->enabled=enabled;
}

static QList<Backend*> backendCache;

QStringList Backend::listAvailableBackends()
{
    QList<Backend* > backends=availableBackends();
    QStringList l;
    foreach(Backend* b, backends)
    {
        if(b->isEnabled())
            l<<b->name();
    }

    return l;
}

QList<Backend*> Backend::availableBackends()
{
    //if we already have all backends Cached, just return the cache.
    //otherwise create the available backends
    if(!backendCache.isEmpty())
    {
        return backendCache;
    }

    KService::List services;
    KServiceTypeTrader* trader = KServiceTypeTrader::self();
    QString error;

    services = trader->query(QLatin1String("Cantor/Backend"));

    foreach (const KService::Ptr &service,  services)
    {
        Backend* backend=service->createInstance<Backend>(0, QVariantList(),  &error);
        if(backend==0)
        {
            qDebug()<<"error: "<<error;
            continue;
        }

        KPluginInfo info(service);
        backend->d->name=info.name();
        backend->d->comment=info.comment();
        backend->d->icon=info.icon();
        backend->d->url=info.website();
        backendCache<<backend;
    }
    return backendCache;
}

Backend* Backend::createBackend(const QString& name)
{
    QList<Backend*> backends=availableBackends();
    foreach(Backend* b, backends)
    {
        if(b->name().toLower()==name.toLower() || b->id().toLower()==name.toLower())
            return b;
    }

    return 0;
}

QWidget* Backend::settingsWidget(QWidget* parent) const
{
    Q_UNUSED(parent)
        return 0;
}

KConfigSkeleton* Backend::config() const
{
    return 0;
}


QStringList Backend::extensions() const
{
    QList<Extension*> extensions=findChildren<Extension*>(QRegExp(QLatin1String(".*Extension")));
    QStringList names;
    foreach(Extension* e, extensions)
        names<<e->objectName();
    return names;
}

Extension* Backend::extension(const QString& name) const
{
    return findChild<Extension*>(name);
}

bool Backend::requirementsFullfilled() const
{
    return true;
}
