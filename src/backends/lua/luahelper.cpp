/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#include "luahelper.h"
#include "luakeywords.h"

#include <lua.hpp>
#include <QRegularExpression>
#include <QString>

QString luahelper_tostring(lua_State* L, int idx)
{
    lua_getglobal(L, "tostring");
    lua_pushvalue(L, idx - 1);      // tostring is on the top now!
    lua_call(L, 1, 1);
    QString str = QString::fromUtf8(lua_tostring(L, -1));
    lua_pop(L, 1);
    return str;
}

QString luahelper_dostring(lua_State* L, const QString& str)
{
    const QByteArray arr = str.toUtf8();
    bool err = ( luaL_loadbuffer(L, arr.data(), arr.size(), nullptr) || lua_pcall (L, 0, LUA_MULTRET, 0) );
    QString ret;

    if(err)
    {
        ret = QString::fromUtf8(lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    return  ret;
}

QString luahelper_getprinted(lua_State* L)
{
    luaL_loadstring(L, "return table.concat(__cantor, '\\n')");
    QString printed;

    if(!lua_pcall(L, 0, 1, 0) )
        printed = QString::fromUtf8(lua_tostring(L, -1));

    lua_pop(L, 1);

    luaL_loadstring(L, "__cantor = {}");        // survives a __cantor = nil!
    if( lua_pcall(L, 0, 0, 0) ) lua_pop(L, 1);

    return printed;
}

static void luahelper_getkeys(lua_State* L, QStringList& list, const QString& prefix = QLatin1String(""))
{
    if(lua_type(L, -1) == LUA_TTABLE)
    {
        // ok, its a table, iterate the keys
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            if(lua_type(L, -2) == LUA_TSTRING)
            {
                QString key = QString::fromUtf8(lua_tostring(L, -2));
                list << prefix + key;
            }
            lua_pop(L, 1);
        }
    }
}

QStringList luahelper_completion(lua_State* L, const QString& name)
{
    int top = lua_gettop(L);

    QStringList list;

    QStringList sections = name.split(QRegularExpression(QStringLiteral("\\.|:")));
    QString table, prefix;

    if(sections.size() == 1)            // global table
    {
        list = LuaKeywords::instance()->keywords();
        table = QLatin1String("_G");
    }
    else
    {
        if(sections.size() == 2)
        {
            table  = sections.first();                          // table.key
            prefix = name.left(sections.first().length() + 1);  // table.
        }
    }

    if(!table.isEmpty())
    {
        // get keys from the table
        QByteArray arr = table.toUtf8();
        lua_getglobal(L, arr.data());
        luahelper_getkeys(L, list, prefix);

        // get keys from the metatable.__index
        if( lua_getmetatable(L, -1) )
        {
            lua_getfield (L, -1, "__index");
            luahelper_getkeys(L, list, prefix);
            lua_pop(L, 2);
            // pop metatable and metatable.__index
        }

        lua_pop(L, 1); // pop table
    }

    lua_settop(L, top);

    return list;
}
