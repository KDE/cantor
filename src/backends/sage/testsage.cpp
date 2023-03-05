/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "testsage.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"
#include "textresult.h"

#include <QDebug>

QString TestSage::backendName()
{
    return QLatin1String("sage");
}

void TestSage::initTestCase() {
    if (QStandardPaths::findExecutable(QLatin1String("sage")).isEmpty())
        QSKIP("Sage executable not found");
    BackendTest::initTestCase();
}

void TestSage::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );
    QCOMPARE( e->result()->data().toString(), QLatin1String(("4")) );
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

    QCOMPARE(cleanOutput(e1->result()->data().toString()), QLatin1String("1"));
    QCOMPARE(cleanOutput(e2->result()->data().toString()), QLatin1String("2"));
    QCOMPARE(cleanOutput(e3->result()->data().toString()), QLatin1String("3"));
}

void TestSage::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2 \n simplify(1 - x + x)") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );
    QCOMPARE( e->result()->data().toString(), QLatin1String("4\n1") );
}

void TestSage::testDefineFunction()
{
    const char* cmd="def func1(param) : \n" \
                    "    return param*param\n\n";

    Cantor::Expression* e1=evalExp( QLatin1String(cmd) );

    QVERIFY( e1!=nullptr );

    Cantor::Expression* e2=evalExp( QLatin1String("func1(2)") );
    QVERIFY( e2!=nullptr );
    QVERIFY( e2->result()!=nullptr );
    QCOMPARE( e2->result()->data().toString(), QLatin1String("4") );
}

void TestSage::testPlot()
{
    Cantor::Expression* e=evalExp( QLatin1String("plot(sin(x))") );

    QVERIFY( e!=nullptr );
    QCOMPARE( e->results().size(), 2);
    QVERIFY( e->results()[0]->type() == Cantor::TextResult::Type);
    QCOMPARE( e->results()[0]->data().toString(), QLatin1String("Launched png viewer for Graphics object consisting of 1 graphics primitive"));

    QVERIFY( e->results()[1]->type() == Cantor::ImageResult::Type);
    QVERIFY( !e->results()[1]->data().isNull() );
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
    QCOMPARE( e->result()->data().toString(), QLatin1String("2") );
}

void TestSage::testLoginLogout()
{
    // Logout from session twice and all must works fine
    session()->logout();
    session()->logout();

    // Login in session twice and all must works fine
    session()->login();
    session()->login();
}

void TestSage::testRestartWhileRunning()
{
    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String("import time; time.sleep(5)"));

    session()->logout();
    QCOMPARE(e1->status(), Cantor::Expression::Interrupted);
    session()->login();

    Cantor::Expression* e2=evalExp( QLatin1String("2+2") );

    QVERIFY(e2 != nullptr);
    QVERIFY(e2->result() != nullptr);

    QCOMPARE(e2->result()->data().toString(), QLatin1String("4"));
}


QTEST_MAIN( TestSage )

