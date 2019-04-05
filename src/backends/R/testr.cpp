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
    Copyright (C) 2019 Sirgienko Nikita <warquark@gmail.com>
 */

#include "testr.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "completionobject.h"
#include "defaultvariablemodel.h"

#include <QDebug>


QString TestR::backendName()
{
    return QLatin1String("R");
}

void TestR::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput( e->result()->data().toString() ), QLatin1String("[1] 4") );
}

void TestR::testMultilineCommand()
{
    Cantor::Expression* e = evalExp(QLatin1String("print(2+2)\nprint(7*5)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(cleanOutput( e->result()->data().toString() ), QLatin1String("[1] 4\n[1] 35"));
}

void TestR::testCodeWithComments()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2 #comment") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput( e->result()->data().toString() ), QLatin1String("[1] 4") );
}

void TestR::testCommandQueue()
{
    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String("0+1"));
    Cantor::Expression* e2=session()->evaluateExpression(QLatin1String("1+1"));
    Cantor::Expression* e3=evalExp(QLatin1String("1+2"));

    qDebug() << e1 << e1->command() << e1->status();
    qDebug() << e2 << e2->command() << e2->status();
    qDebug() << e3 << e3->command() << e3->status();

    QVERIFY(e1!=nullptr);
    QVERIFY(e2!=nullptr);
    QVERIFY(e3!=nullptr);

    QVERIFY(e1->result());
    QVERIFY(e2->result());
    QVERIFY(e3->result());

    QCOMPARE(cleanOutput(e1->result()->data().toString()), QLatin1String("[1] 1"));
    QCOMPARE(cleanOutput(e2->result()->data().toString()), QLatin1String("[1] 2"));
    QCOMPARE(cleanOutput(e3->result()->data().toString()), QLatin1String("[1] 3"));
}

void TestR::testVariablesCreatingFromCode()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    Cantor::Expression* e=evalExp(QLatin1String("a1 = 15; b1 = 'S'; d1 = c(1,2,3)"));

    QVERIFY(e!=nullptr);
    QVERIFY(e->result() != nullptr);

    while (session()->status() != Cantor::Session::Done)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(model->rowCount(), 3);

    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("a1"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("15"));

    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("b1"));
    QCOMPARE(model->index(1,1).data().toString(), QLatin1String("S"));

    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("d1"));
    QCOMPARE(model->index(2,1).data().toString(), QLatin1String("1, 2, 3"));

    evalExp(QLatin1String("rm(a1,b1,d1)"));
}

void TestR::testVariableCleanupAfterRestart()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    while(session()->status() != Cantor::Session::Done)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(model->rowCount(), 0);

    Cantor::Expression* e=evalExp(QLatin1String("h1 = 15; h2 = 'S';"));
    QVERIFY(e!=nullptr);

    while (session()->status() != Cantor::Session::Done)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(model->rowCount(), 2);

    session()->logout();
    session()->login();

    QCOMPARE(model->rowCount(), 0);
}

void TestR::testVariableDefinition()
{
    Cantor::Expression* e = evalExp(QLatin1String("testvar <- \"value\"; testvar"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("[1] \"value\""));

    evalExp(QLatin1String("rm(testvar)"));
}

void TestR::testMatrixDefinition()
{
    Cantor::Expression* e = evalExp(QLatin1String("matrix(1:9, nrow = 3, ncol = 3)"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->data().toString(), QLatin1String(
         "     [,1] [,2] [,3]\n"
         "[1,]    1    4    7\n"
         "[2,]    2    5    8\n"
         "[3,]    3    6    9"
    ));
}

void TestR::testCommentExpression()
{
    Cantor::Expression* e = evalExp(QLatin1String("#only comment"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QCOMPARE(e->results().size(), 0);
}

void TestR::testMultilineCommandWithComment()
{
    Cantor::Expression* e = evalExp(QLatin1String(
        "print(2+2) \n"
        "#comment in middle \n"
        "print(7*5)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result()->data().toString() == QLatin1String("[1] 4\n[1] 35"));
}

void TestR::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2*+?!<") );

    QVERIFY( e!=nullptr );
    QCOMPARE( e->status(), Cantor::Expression::Error );
}

void TestR::testCompletion()
{
    Cantor::CompletionObject* help = session()->completionFor(QLatin1String("pi"), 2);
    waitForSignal(help, SIGNAL(fetchingDone()));

    // Checks all completions for this request
    // This correct for R 3.4.4
    const QStringList& completions = help->completions();
    qDebug() << completions;
    QCOMPARE(completions.size(), 5);
    QVERIFY(completions.contains(QLatin1String("pi")));
    QVERIFY(completions.contains(QLatin1String("pico")));
    QVERIFY(completions.contains(QLatin1String("pictex")));
    QVERIFY(completions.contains(QLatin1String("pie")));
    QVERIFY(completions.contains(QLatin1String("pipe")));
}

QTEST_MAIN( TestR )

