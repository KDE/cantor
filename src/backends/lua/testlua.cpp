/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
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
