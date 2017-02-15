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

#ifndef _LUASESSION_H
#define _LUASESSION_H

#include "session.h"
#include <lua.hpp>

class LuaExpression;

class LuaSession : public Cantor::Session
{
  Q_OBJECT
public:
    LuaSession( Cantor::Backend* backend);
    ~LuaSession();

    void login();
    void logout();

    void interrupt();

    Cantor::Expression*         evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
    Cantor::CompletionObject*   completionFor(const QString& cmd, int index=-1);
    virtual QSyntaxHighlighter* syntaxHighlighter(QObject* parent);
    lua_State*                  getState() const;

private Q_SLOTS:
    void expressionFinished();

private:
    lua_State* m_L;
};

#endif /* _LUASESSION_H */
