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

#ifndef _LUAEXPRESSION_H
#define _LUAEXPRESSION_H

#include "expression.h"

struct lua_State;

class LuaExpression : public Cantor::Expression
{
    Q_OBJECT

public:
    LuaExpression( Cantor::Session* session);
    ~LuaExpression() override = default;

    void evaluate() override;
    void interrupt() override;
    void parseOutput(QString& output);
    void parseError(QString& error);

};

#endif /* _LUAEXPRESSION_H */
