/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include <vector>

#include "backend.h"
#include "extension.h"

#include <QDir>
#include <QRegularExpression>
#include <QUrl>
#include <QProcess>
#include <QStandardPaths>
#include <QPluginLoader>

#include <KPluginFactory>
#include <KPluginMetaData>
#include <KPluginFactory>
#include <KLocalizedString>

using namespace Cantor;

class Cantor::BackendPrivate
{
  public:
    QString name;
    QString comment;
    QString icon;
    QString url;
    bool enabled{true};
    QList<GraphicPackage> supportedGraphicPackagesCache;
};

Backend::Backend(QObject* parent, const QList<QVariant>& args) : QObject(parent),
                                                                d(new BackendPrivate)
{
    Q_UNUSED(args)
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

QString Backend::defaultHelp() const
{
    return QString();
}

bool Backend::isEnabled() const
{
    return d->enabled && requirementsFullfilled();
}

void Backend::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

QStringList Backend::listAvailableBackends()
{
    QStringList l;
    for (Backend* b : availableBackends())
    {
        if(b->isEnabled())
            l<<b->name();
    }

    return l;
}

QList<Backend*> Backend::availableBackends()
{
    static QList<Backend*> backendCache;
    //if we already have all backends Cached, just return the cache.
    //otherwise create the available backends
    if(!backendCache.isEmpty())
    {
        return backendCache;
    }

    const QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("cantor_backends"));

    for (const KPluginMetaData &plugin : plugins) {

        const auto result = KPluginFactory::instantiatePlugin<Backend>(plugin, QCoreApplication::instance());

        if (!result) {
            qDebug() << "Error while loading backend: " << result.errorText;
            continue;
        }

        Backend *backend = result.plugin;

        backend->d->name = plugin.name();
        backend->d->comment = plugin.description();
        backend->d->icon = plugin.iconName();
        backend->d->url = plugin.website();
        backendCache << backend;
    }

    return backendCache;
}

Backend* Backend::getBackend(const QString& name)
{
    for (Backend* b : availableBackends())
    {
        if(b->name().toLower()==name.toLower() || b->id().toLower()==name.toLower())
            return b;
    }

    return nullptr;
}

QStringList Backend::extensions() const
{
    QList<Extension*> extensions = findChildren<Extension*>(QRegularExpression(QLatin1String(".*Extension")));
    QStringList names;
    for (Extension* e : extensions)
        names << e->objectName();
    return names;
}

Extension* Backend::extension(const QString& name) const
{
    return findChild<Extension*>(name);
}

bool Backend::checkExecutable(const QString& name, const QString& path, QString* reason)
{
    if (path.isEmpty())
    {
        if (reason)
            *reason = i18n("No path for the %1 executable specified. "
                        "Please provide the correct path in the application settings and try again.",
                        name);
        return false;
    }

    QFileInfo info(path);
    if (!info.exists())
    {
        if (reason)
            *reason = i18n("The specified file '%1' for the %2 executable doesn't exist. "
                        "Please provide the correct path in the application settings and try again.",
                        path, name);
        return false;
    }

    if (!info.isExecutable())
    {
        if (reason)
            *reason = i18n("The specified file '%1' doesn't point to an executable. "
                        "Please provide the correct path in the application settings and try again.",
                        path);
        return false;
    }

    return true;
}

bool Cantor::Backend::testProgramWritable(const QString& program, const QStringList& args, const QString& filename, const QString& expectedContent, QString* reason, int timeOut)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(args);
    process.start();

    if (process.waitForFinished(timeOut) == false)
    {
        if (reason)
            *reason = i18n("The program %1 didn't finish the execution after %2 milliseconds during the plot integration test.", QFileInfo(program).fileName(), timeOut);

        return false;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        if (reason)
            *reason = i18n("Failed to open the file %1 during the plot integration test.", filename);
        return false;
    }

    QString fileContent = QString::fromLocal8Bit(file.readAll());
    if (fileContent.trimmed() != expectedContent)
    {
        if (reason)
            *reason = i18n("Failed to parse the result during the plot integration test.");
        return false;
    }

    file.close();
    file.remove();

    return true;
}

QList<GraphicPackage> Backend::availableGraphicPackages() const
{
    if (d->supportedGraphicPackagesCache.size() != 0)
        return d->supportedGraphicPackagesCache;

    if (!(capabilities() & Capability::IntegratedPlots))
        return QList<GraphicPackage>(); // because this cache is empty

    QString packagesFile = id() + QLatin1String("/graphic_packages.xml");
    QString filename = QStandardPaths::locate(QStandardPaths::AppDataLocation, packagesFile, QStandardPaths::LocateFile);

    if (filename.isEmpty())
        filename = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("cantor/") + packagesFile, QStandardPaths::LocateFile);

    if (filename.isEmpty())
        return QList<GraphicPackage>();

    d->supportedGraphicPackagesCache = GraphicPackage::loadFromFile(filename);

    return d->supportedGraphicPackagesCache;
}
