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
    Copyright (C) 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
 */

#include "luabackend.h"
#include "luasession.h"
#include "luaextensions.h"
#include "cantor_macros.h"

#include "settings.h"
#include "ui_settings.h"

#include <QFileInfo>

LuaBackend::LuaBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    setObjectName(QLatin1String("LuaBackend"));
    new LuaScriptExtension(this);
}

QString LuaBackend::id() const
{
    return QLatin1String("lua");
}

QString LuaBackend::version() const
{
    return QLatin1String("LuaJIT 2.0");
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
    const QString& replPath = LuaSettings::self()->path().toLocalFile();
    if (replPath.isEmpty())
    {
        if (reason)
            *reason = i18n("Lua backend needs installed Lua programming language. The backend often automatically founds needed Lua binary file, but not in this case. Please, go to Cantor settings and set path to Lua executable");
        return false;
    }

    QFileInfo info(replPath);
    if (info.isExecutable())
        return true;
    else
    {
        if (reason)
            *reason = i18n("In Lua backend settings a path to Lua binary file set as %1, but this file not executable. Do you sure, that this is correct path to Lua? Change this path in Cantor settings, if no.").arg(replPath);
        return false;
    }
}

QUrl LuaBackend::helpUrl() const
{
    return QUrl(i18nc("Lua official documentation", "http://www.lua.org/docs.html"));
}

QString LuaBackend::description() const
{
    return i18n("<p> Lua is a fast and lightweight scripting language, with a simple procedural syntax." \
                " There are several libraries in Lua aimed at math and science.</p>"
                "<p>This backend supports luajit 2.</p>");
}

QWidget* LuaBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget = new QWidget(parent);
    Ui::LuaSettingsBase s;
    s.setupUi(widget);

    return widget;
}

KConfigSkeleton* LuaBackend::config() const
{
    return LuaSettings::self();
}

K_PLUGIN_FACTORY_WITH_JSON(luabackend, "luabackend.json", registerPlugin<LuaBackend>();)
#include "luabackend.moc"
