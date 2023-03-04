/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
*/

#include "backend.h"
#include "expression.h"
#include "imageresult.h"
#include "result.h"
#include "session.h"
#include "testscilab.h"

#include <QtTest>

QString TestScilab::backendName()
{
    return QLatin1String("scilab");
}

void TestScilab::initTestCase() {
    if (QStandardPaths::findExecutable(QLatin1String("scilab")).isEmpty())
        QSKIP("Scilab executable not found");
    BackendTest::initTestCase();
}

void TestScilab::testSimpleCommand()
{
    Cantor::Expression* e = evalExp(QLatin1String("printf(\"Testing\")\n"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("Testing"));
}

void TestScilab::testVariableDefinition()
{
    Cantor::Expression* e = evalExp(QLatin1String("test = 100 disp(test)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("100"));
}

void TestScilab::testInvalidSyntax()
{
    Cantor::Expression* e = evalExp(QLatin1String("^invalidvar = 100")); // invalid variable name, since it starts with ^

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
}

void TestScilab::testLoginLogout()
{
    // Logout from session twice and all must works fine
    session()->logout();
    session()->logout();

    // Login in session twice and all must works fine
    session()->login();
    session()->login();
}

void TestScilab::testPlot()
{
    Cantor::Expression* e = evalExp(QLatin1String("x = [0:0.1:2*%pi] plot(x, sin(x))"));

    int cnt = 0;
    //give some time to create the image
    while(e->result() == nullptr || e->result()->type() != (int)Cantor::ImageResult::Type)
    {
        QTest::qWait(250);
        cnt += 250;
        if(cnt > 5000)
            break;
    }

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);

    QCOMPARE(e->result()->type(), (int)Cantor::ImageResult::Type);

    QVERIFY(!e->result()->data().isNull());
    QVERIFY(e->errorMessage().isNull());
}

QTEST_MAIN(TestScilab)

