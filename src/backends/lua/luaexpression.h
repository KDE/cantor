/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#ifndef _LUAEXPRESSION_H
#define _LUAEXPRESSION_H

#include "expression.h"

class LuaExpression : public Cantor::Expression
{
    Q_OBJECT

public:
    explicit LuaExpression( Cantor::Session*, bool internal = false);
    ~LuaExpression() override = default;

    void evaluate() override;
    void parseOutput(const QString&) override;
};

#endif /* _LUAEXPRESSION_H */
