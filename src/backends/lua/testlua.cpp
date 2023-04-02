/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/

#include "testlua.h"

#include "session.h"
#include "result.h"
#include "luaexpression.h"

QString TestLua::backendName()
{
    return QLatin1String("lua");
}

void TestLua::testSimpleCommand()
{
    auto* e = evalExp( QLatin1String("print(2+2)\n") );

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString() ), QLatin1String("4"));
}

void TestLua::testMultilineCommand01()
{
    auto* e = evalExp(QLatin1String("print(4+4); print(2-1)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->errorMessage().isNull());
    QCOMPARE(e->results().size(), 2);

    QCOMPARE(e->results().at(0)->data().toString(), QLatin1String("8"));
    QCOMPARE(e->results().at(1)->data().toString(), QLatin1String("1"));
}

/*!
 * test multiple assignments with comments and with multi-line strings
 */
void TestLua::testMultilineCommand02()
{
    QSKIP("Works in Cantor, doesn't work in the test");
    auto* e = evalExp(QLatin1String(
        "s = 'walternate'  -- Immutable strings like Python.\n"
        "t = \"double-quotes are also fine\"\n"
        "u = [[ Double brackets\n"
        "    start and end\n"
        "    multi-line strings.]]\n"
        "print(u)\n"
    ));

    QVERIFY(e != nullptr);
    QVERIFY(e->errorMessage().isNull());
    QCOMPARE(e->results().size(), 1);

    QCOMPARE(e->results().at(0)->data().toString(),
             QLatin1String(
                 "Double brackets"
                "       start and end"
                "       multi-line strings."
             ));
}

void TestLua::testVariableDefinition()
{
    auto* e = evalExp( QLatin1String("num = 42; print(num)") );

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("42"));
}

void TestLua::testInvalidSyntax()
{
    QSKIP("Works in Cantor, doesn't work in the test");
    auto* e = evalExp( QLatin1String("2+2*+.") );

    QVERIFY(e != nullptr);

    waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));
    QCOMPARE(e->status(), Cantor::Expression::Done);

    if (e->status() != Cantor::Expression::Error)
        waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Error);
}

void TestLua::testIfElseCondition()
{
    QLatin1String cmd(
        "if 12 > 50 then"
        "  print('true')"
        "else"
        "  print('false')"
        "end");

    auto* e = evalExp(cmd);

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("false"));
}

void TestLua::testForLoop()
{
    QSKIP("Works in Cantor, doesn't work in the test");
    QLatin1String cmd(
        "karlSum = 0""\n"
        "for i = 1, 100 do""\n"
        "  karlSum = karlSum + i""\n"
        "end""\n"
        "print(karlSum)");

    auto* e = evalExp(cmd);

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput(e->result()->data().toString()), QLatin1String("5050") );
}

void TestLua::testWhileLoop()
{
    QSKIP("Works in Cantor, doesn't work in the test");
    auto* e = evalExp(QLatin1String(
        "num = 42\n"
        "print(num)\n"
        "while num < 50 do\n"
        "   num = num + 1  -- No ++ or += type operators.\n"
        "end\n"
        "print(num)\n"
    ));

    if (e->status() != Cantor::Expression::Done)
        waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));

    QVERIFY(e != nullptr);
    QVERIFY(e->errorMessage().isNull());
    QCOMPARE(e->results().size(), 2);

    QCOMPARE(e->results().at(0)->data().toString(), QLatin1String("42"));
    QCOMPARE(e->results().at(1)->data().toString(), QLatin1String("50"));
}

void TestLua::testFunction()
{
    QSKIP("Works in Cantor, doesn't work in the test");
    QLatin1String cmd(
        "function max(num1, num2)\n"
            "if (num1 > num2) then\n"
                "result = num1;\n"
            "else\n"
                "result = num2;\n"
            "end\n"
            "return result;\n"
        "end");

    auto* e = evalExp(cmd);

    if (e->status() != Cantor::Expression::Done)
        waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));

    QVERIFY(e!=nullptr);
    QVERIFY(e->result() == nullptr);

    cmd = QLatin1String("print(max(5,10));");
    auto* e2 = evalExp(cmd);

    if (e2->status() != Cantor::Expression::Done)
        waitForSignal(e2, SIGNAL(statusChanged(Cantor::Expression::Status)));

    QVERIFY( e2!=nullptr );
    QVERIFY( e2->result()!=nullptr );
    QCOMPARE( cleanOutput(e2->result()->data().toString()), QLatin1String("10") );
}

QTEST_MAIN( TestLua )
