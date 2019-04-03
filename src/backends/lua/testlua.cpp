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
    Copyright (C) 2018 Nikita Sirgienko <warquark@gmail.com>
 */

#include "testlua.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"
#include "epsresult.h"

#include "luaexpression.h"

#include <QDebug>

QString TestLua::backendName()
{
    return QLatin1String("lua");
}

void TestLua::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("print(2+2)\n") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput( e->result()->data().toString() ), QLatin1String("4") );
}

void TestLua::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("print(4+4); print(2-1)") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput(e->result()->data().toString()), QLatin1String("8\n1") );
}

void TestLua::testVariableDefinition()
{
    Cantor::Expression* e=evalExp( QLatin1String("num = 42; print(num)") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput(e->result()->data().toString()), QLatin1String("42") );
}

void TestLua::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2*+.") );

    QVERIFY( e!=nullptr );
    QCOMPARE( e->status(), Cantor::Expression::Error );
}

void TestLua::testIfElseCondition()
{
    QSKIP("Skip, until problem with multiline input for lua backends not will be solved");
    QLatin1String cmd(
        "if 12 > 50 then"
        "  print('true')"
        "else"
        "  print('false')"
        "end");

    Cantor::Expression* e=evalExp(cmd);

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput(e->result()->data().toString()), QLatin1String("false") );
}

void TestLua::testForLoop()
{
    QSKIP("Skip, until problem with multiline input for lua backends not will be solved");
    QLatin1String cmd(
        "karlSum = 0""\n"
        "for i = 1, 100 do""\n"
        "  karlSum = karlSum + i""\n"
        "end""\n"
        "print(karlSum)");

    Cantor::Expression* e=evalExp(cmd);

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput(e->result()->data().toString()), QLatin1String("5050") );
}

QTEST_MAIN( TestLua )
