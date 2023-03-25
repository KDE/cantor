/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
*/

#include "luabackend.h"
#include "luaextensions.h"
#include "luasession.h"
#include "luasettingswidget.h"
#include "settings.h"

#include <KPluginFactory>

LuaBackend::LuaBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    new LuaScriptExtension(this);
}

QString LuaBackend::id() const
{
    return QLatin1String("lua");
}

QString LuaBackend::version() const
{
    return QLatin1String("LuaJIT 2.1");
}

Cantor::Session* LuaBackend::createSession()
{
    return new LuaSession(this);
}

Cantor::Backend::Capabilities LuaBackend::capabilities() const
{
    Cantor::Backend::Capabilities cap =
        Cantor::Backend::SyntaxHighlighting |
        Cantor::Backend::Completion;

    return cap;
}

bool LuaBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = LuaSettings::self()->path().toLocalFile();
    return Cantor::Backend::checkExecutable(QLatin1String("Lua"), path, reason);
}

QUrl LuaBackend::helpUrl() const
{
    return QUrl(i18nc("Lua official documentation", "https://www.lua.org/docs.html"));
}

QString LuaBackend::description() const
{
    return i18n("<b>Lua</b> is a fast and lightweight scripting language, with a simple procedural syntax." \
                " There are several libraries in Lua aimed at math and science.");
}

QWidget* LuaBackend::settingsWidget(QWidget* parent) const
{
    return new LuaSettingsWidget(parent, id());
}

KConfigSkeleton* LuaBackend::config() const
{
    return LuaSettings::self();
}

K_PLUGIN_FACTORY_WITH_JSON(luabackend, "luabackend.json", registerPlugin<LuaBackend>();)
#include "luabackend.moc"
