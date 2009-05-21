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
using namespace MathematiK;

#include <kservicetypetrader.h>
#include <kservice.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <kplugininfo.h>

#include "extension.h"

class MathematiK::BackendPrivate
{
  public:
    QString name;
    QString description;
    QString icon;
    bool enabled;
};

Backend::Backend(QObject* parent, const QList<QVariant> args) : QObject(parent),
                                                                d(new BackendPrivate)
{
    Q_UNUSED(args)
}

Backend::~Backend()
{

}

QString Backend::name()
{
    return d->name;
}

QString Backend::description()
{
    return d->description;
}

QString Backend::icon()
{
    return d->icon;
}

bool Backend::isEnabled()
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
        l<<b->name();

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

    services = trader->query("MathematiK/Backend");

    foreach (KService::Ptr service,  services)
    {
        Backend* backend=service->createInstance<Backend>(0, QVariantList(),  &error);
        if(backend==0)
            kDebug()<<"error: "<<error;

        KPluginInfo info(service);
        backend->d->name=info.name();
        backend->d->description=info.comment();
        backend->d->icon=info.icon();
        backendCache<<backend;
    }
    return backendCache;
}

Backend* Backend::createBackend(const QString& name)
{
    QList<Backend*> backends=availableBackends();
    foreach(Backend* b, backends)
    {
        if(b->name()==name)
            return b;
    }

    return 0;
}

QWidget* Backend::settingsWidget(QWidget* parent)
{
    Q_UNUSED(parent)
        return 0;
}

KConfigSkeleton* Backend::config()
{
    return 0;
}


QStringList Backend::extensions()
{
    QList<Extension*> extensions=findChildren<Extension*>(QRegExp(".*Extension"));
    QStringList names;
    foreach(Extension* e, extensions)
        names<<e->objectName();
    return names;
}

Extension* Backend::extension(const QString& name)
{
    return findChild<Extension*>(name);
}

bool Backend::requirementsFullfilled()
{
    return true;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Backend::Capabilities)


