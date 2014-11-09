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

#include "luasession.h"
#include "luaexpression.h"
#include "luacompletionobject.h"
#include "luahighlighter.h"
#include "luahelper.h"
#include <settings.h>
#include "ui_settings.h"

LuaSession::LuaSession( Cantor::Backend* backend) : Session(backend)
{
}

LuaSession::~LuaSession()
{
}

void LuaSession::login()
{
    m_L = luaL_newstate();
    luaL_openlibs(m_L);

    QStringList errors;
    errors << luahelper_dostring(m_L, QLatin1String("__cantor = {}"));

    errors << luahelper_dostring(m_L,
        QLatin1String("function print(...)\n"
            "local t = {}\n"
            "for i = 1, select('#',...) do\n"
                "local a = select(i,...)\n"
                "t[i] = tostring(a)\n"
            "end\n"
            "table.insert(__cantor, table.concat(t,'\t'))\n"
        " end"));

    errors << luahelper_dostring(m_L,
        QLatin1String("function show(a)\n"
            "assert(type(a) == 'string')\n"
            "return a\n"
        "end"));

    if(!errors.empty())
        qDebug() << errors.join(QLatin1String("\n"));

    foreach (const QString &str, LuaSettings::self()->autorunScripts())
        evaluateExpression(QLatin1String("dofile('") + str + QLatin1String("')"), Cantor::Expression::DeleteOnFinish);

    changeStatus(Cantor::Session::Done);
    emit ready();
}

void LuaSession::logout()
{
    if(m_L)
    {
        lua_close(m_L);
        m_L = 0;
    }
}

void LuaSession::interrupt()
{
    // Lua backend is synchronous, there is no way to currently interrupt an expression (in a clean way)
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* LuaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    changeStatus(Cantor::Session::Running);

    LuaExpression* expr = new LuaExpression(this, m_L);
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionFinished()));
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

Cantor::CompletionObject* LuaSession::completionFor(const QString& command, int index)
{
    return new LuaCompletionObject(command, index, this);
}

void LuaSession::expressionFinished()
{
    // synchronous
    changeStatus(Cantor::Session::Done);
}

QSyntaxHighlighter* LuaSession::syntaxHighlighter(QObject* parent)
{
    LuaHighlighter* highlighter = new LuaHighlighter(parent);
    return highlighter;
}

lua_State* LuaSession::getState() const
{
    return m_L;
}

#include "luasession.moc"
