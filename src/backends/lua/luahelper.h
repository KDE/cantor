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

#ifndef _LUAHELPER_H
#define _LUAHELPER_H

struct lua_State;
class  QString;
class  QStringList;

/* follows lua_funcname convention */
QString     luahelper_tostring   (lua_State* L, int idx);
QString     luahelper_dostring   (lua_State* L, const QString& str);
QString     luahelper_getprinted (lua_State* L);
QStringList luahelper_completion (lua_State* L, const QString& name);

/* compile-time */
QStringList luahelper_keywords   ();
QStringList luahelper_functions  ();
QStringList luahelper_variables  ();

#endif /* _LUAHELPER_H */
