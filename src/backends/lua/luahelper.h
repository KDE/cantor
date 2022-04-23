/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#ifndef _LUAHELPER_H
#define _LUAHELPER_H

#include <QStringList>

struct lua_State;
class  QString;

/* follows lua_funcname convention */
QString     luahelper_tostring   (lua_State* L, int idx);
QString     luahelper_dostring   (lua_State* L, const QString& str);
QString     luahelper_getprinted (lua_State* L);
QStringList luahelper_completion (lua_State* L, const QString& name);

#endif /* _LUAHELPER_H */
