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

#ifndef LUAEXTENSIONS_H
#define LUAEXTENSIONS_H

#include <extension.h>

#define LUA_EXT_CDTOR_DECL(name) Lua##name##Extension(QObject* parent); \
                                     ~Lua##name##Extension();

class LuaScriptExtension : public Cantor::ScriptExtension
{
public:
    LUA_EXT_CDTOR_DECL(Script)
    virtual QString scriptFileFilter();
    virtual QString highlightingMode();
    virtual QString runExternalScript(const QString& path);
    virtual QString commandSeparator();
};

#endif // LUAEXTENSIONS_H
