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

#include "luaexpression.h"
#include "luasession.h"
#include "luahelper.h"

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"

#include <luajit-2.0/lua.hpp> // need the luajit-2.0 prefix to avoid conflicts with Lua 5.2
#include <kdebug.h>

#include <QString>
#include <QStringList>

LuaExpression::LuaExpression( Cantor::Session* session, lua_State* L)
    : Cantor::Expression(session), m_L(L)
{
}

LuaExpression::~LuaExpression()
{
}

void LuaExpression::evaluate()
{
    QString ret;
    Cantor::Expression::Status status;
    execute(ret, status);

    if(status == Cantor::Expression::Done)
    {
        QString cmd = command().simplified();

        if( cmd.startsWith("show(") || cmd.startsWith("show (") )
            setResult(new Cantor::ImageResult(ret,ret));
        else
            setResult(new Cantor::TextResult(ret));
    }
    else
    {
        setErrorMessage(ret);
    }

    setStatus(status);
}

void LuaExpression::interrupt()
{
    setStatus(Cantor::Expression::Interrupted);
}

void LuaExpression::execute(QString& ret, Cantor::Expression::Status& status)
{
    int top = lua_gettop(m_L);

    // execute the command
    QString err = luahelper_dostring(m_L, "return " + command() ); // try to return values...
    if( !err.isNull() ) err = luahelper_dostring(m_L, command() ); // try the original expression

    if( err.isNull() )
    {
        QStringList list;
        int n_out = lua_gettop(m_L) - top;

        for(int i = -n_out; i < 0; ++i)
            list << luahelper_tostring(m_L, i);

        ret    = list.join("\n") + luahelper_getprinted(m_L);
        status = Cantor::Expression::Done;
    }
    else
    {
        kDebug() << "error when executing" << command() << ":" << err;
        ret    = err;
        status = Cantor::Expression::Error;
    }

    lua_settop(m_L, top);
}

#include "luaexpression.moc"
