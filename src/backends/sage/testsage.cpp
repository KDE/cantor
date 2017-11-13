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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "testsage.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"

#include <QDebug>

QString TestSage::backendName()
{
    return QLatin1String("sage");
}

void TestSage::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );
    QCOMPARE( e->result()->toHtml(), QLatin1String(("4")) );
}

void TestSage::testCommandQueue()
{
    //only wait for the last Expression to return, so the queue gets
    //actually filled

    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String("0+1"));
    Cantor::Expression* e2=session()->evaluateExpression(QLatin1String("1+1"));
    Cantor::Expression* e3=evalExp(QLatin1String("1+2"));

    QVERIFY(e1!=nullptr);
    QVERIFY(e2!=nullptr);
    QVERIFY(e3!=nullptr);

    QVERIFY(e1->result());
    QVERIFY(e2->result());
    QVERIFY(e3->result());

    QCOMPARE(cleanOutput(e1->result()->toHtml()), QLatin1String("1"));
    QCOMPARE(cleanOutput(e2->result()->toHtml()), QLatin1String("2"));
    QCOMPARE(cleanOutput(e3->result()->toHtml()), QLatin1String("3"));
}

void TestSage::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2 \n simplify(1 - x + x)") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );
    QCOMPARE( e->result()->toHtml(), QLatin1String("4<br/>\n1") );
}

void TestSage::testDefineFunction()
{
    const char* cmd="def func1(param) : \n" \
                    "    return param*param\n\n";

    Cantor::Expression* e1=evalExp( QLatin1String(cmd) );

    QVERIFY( e1!=nullptr );
    QVERIFY( e1->result()!=nullptr );

    Cantor::Expression* e2=evalExp( QLatin1String("func1(2)") );
    QVERIFY( e2!=nullptr );
    QVERIFY( e2->result()!=nullptr );
    QCOMPARE( e2->result()->toHtml(), QLatin1String("4") );
}

void TestSage::testPlot()
{
    Cantor::Expression* e=evalExp( QLatin1String("plot(sin(x))") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );
    QVERIFY( e->result()->type()==Cantor::ImageResult::Type );
    QVERIFY( !e->result()->data().isNull() );
}

void TestSage::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2*(") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->errorMessage()== QLatin1String("Syntax Error") );
}

void TestSage::testNoOutput()
{
    Cantor::Expression* e=evalExp(  QLatin1String("f(x)=x^2+3*x+2\nf(0)") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result() != nullptr );
    QCOMPARE( e->result()->toHtml(), QLatin1String("2") );
}


QTEST_MAIN( TestSage )

