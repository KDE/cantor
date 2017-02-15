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

#include "luahelper.h"

#include <lua.hpp>
#include <QString>
#include <QStringList>

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
    bool err = ( luaL_loadbuffer(L, arr.data(), arr.size(), 0) || lua_pcall (L, 0, LUA_MULTRET, 0) );
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

    QStringList sections = name.split(QRegExp(QLatin1String("\\.|:")));
    QString table, prefix;

    if(sections.size() == 1)            // global table
    {
        list = luahelper_keywords();
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

QStringList luahelper_keywords()
{
    static const char* keywords[] = {
        "and", "break", "do", "else", "elseif", "end", "false", "for",
        "function", "if", "in", "local", "nil", "not", "or", "repeat",
        "return", "then", "true", "until", "while", 0
    };

    QStringList list;

    for(int i = 0; keywords[i]; ++i)
        list << QString::fromLatin1(keywords[i]);

    return list;
}


QStringList luahelper_functions()
{
    static const char* functions[] = {
        "assert","collectgarbage","dofile","error","getfenv","getmetatable","ipairs","load","loadfile",
        "loadstring","module","next","pairs","pcall","print","rawequal","rawget","rawset","require","select",
        "setfenv","setmetatable","tonumber","tostring","type","unpack","xpcall","close","flush",
        "lines","read","seek","setvbuf","write","close","flush","input","lines","open","output","popen",
        "read","tmpfile","type","write",
        "math.abs","acos","asin","atan","atan2","ceil", "cos","cosh","deg","exp","floor","fmod","frexp","ldexp",
        "log","log10","max","min","modf", "pow","rad","random","randomseed","sin","sinh","sqrt","tan","tanh",
        "clock","date","difftime", "execute","exit","getenv","remove","rename","setlocale","time","tmpname",
        "loadlib","byte","char", "dump","find","format","gmatch","gsub","len","lower","match","rep","reverse","sub","upper","concat",
        "insert","maxn","remove","sort",0
    };

    QStringList list;

    for(int i = 0; functions[i]; ++i)
        list << QString::fromLatin1(functions[i]);

    return list;
}


QStringList luahelper_variables()
{
    static const char* variables[] = {"_G", "_VERSION", "pi", "huge", "stdin", "stdout", "stderr", 0};

    QStringList list;

    for(int i = 0; variables[i]; ++i)
        list << QString::fromLatin1(variables[i]);

    return list;
}
