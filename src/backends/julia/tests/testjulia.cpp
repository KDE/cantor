/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "testjulia.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "textresult.h"

QString TestJulia::backendName()
{
    return QLatin1String("julia");
}

void TestJulia::testOneLine()
{
    Cantor::Expression *e = evalExp(QLatin1String("2 + 3"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("5"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testOneLineWithPrint()
{
    Cantor::Expression *e = evalExp(QLatin1String("print(2 + 3)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("5"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testException()
{
    Cantor::Expression *e = evalExp(QLatin1String("sqrt(-1)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QVERIFY(
        not e->errorMessage().isEmpty()
        and e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument. Try sqrt(complex(x))."
        ))
    );
}

void TestJulia::testSyntaxError()
{
    Cantor::Expression *e = evalExp(QLatin1String("for i = 1:10\nprint(i)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QVERIFY(
        not e->errorMessage().isEmpty()
        and e->errorMessage().contains(QLatin1String(
            "syntax: incomplete: \"for\" at none:1 requires end"
        ))
    );
}

void TestJulia::testMultilineCode()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "q = 0; # comment\n"
        "# sdfsdf\n"
        "for i = 1:10\n"
        "    q += i\n"
        "end\n"
        "print(q)"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("55"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testPartialResultOnException()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "for i = 1:5\n"
        "    print(i)\n"
        "end\n"
        "sqrt(-1)\n"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("12345"));
    QVERIFY(
        not e->errorMessage().isEmpty()
        and e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument. Try sqrt(complex(x))."
        ))
    );
}

QTEST_MAIN(TestJulia)

#include "testjulia.moc"
