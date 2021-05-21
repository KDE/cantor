/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#include "luaextensions.h"
#include <KLocalizedString>

LuaScriptExtension::LuaScriptExtension(QObject* parent): Cantor::ScriptExtension(parent) {}

LuaScriptExtension::~LuaScriptExtension() {}

QString LuaScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("dofile(\"%1\")").arg(path);
}

QString LuaScriptExtension::scriptFileFilter()
{
    return i18n("Lua script file (*.lua)");
}

QString LuaScriptExtension::highlightingMode()
{
    return QLatin1String("lua");
}

QString LuaScriptExtension::commandSeparator()
{
    return QLatin1String("");
}
