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

#include "luaextensions.h"
#include <KLocale>

#define LUA_EXT_CDTOR(name) Lua##name##Extension::Lua##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Lua##name##Extension::~Lua##name##Extension() {}

LUA_EXT_CDTOR(Script)

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
