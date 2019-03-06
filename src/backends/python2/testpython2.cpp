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
    Copyright (C) 2013 Tuukka Verho <tuukka.verho@aalto.fi>
 */

#include "testpython2.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "defaultvariablemodel.h"

QString TestPython2::backendName()
{
    return QLatin1String("python2");
}

void TestPython2::testImportNumpy()
{
    Cantor::Expression* e = evalExp(QLatin1String("import numpy"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
}

void TestPython2::testCodeWithComments()
{
    {
    Cantor::Expression* e = evalExp(QLatin1String("#comment\n1+2"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }

    {
    Cantor::Expression* e = evalExp(QLatin1String("     #comment\n1+2"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }
}

void TestPython2::testSimpleCode()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2"));

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QString result=e->result()->toHtml();

    QCOMPARE( cleanOutput(result ), QLatin1String("4") );
}

void TestPython2::testMultilineCode()
{
    Cantor::Expression* e=evalExp(QLatin1String(
        "a = 2+2\n"
        "b = 3+3\n"
        "print a,b"
    ));

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QString result=e->result()->toHtml();

    QCOMPARE( cleanOutput(result ), QLatin1String("4 6") );

    evalExp(QLatin1String("del a; del b"));
}

void TestPython2::testVariablesCreatingFromCode()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    Cantor::Expression* e=evalExp(QLatin1String("a = 15; b = 'S';"));
    QVERIFY(e!=nullptr);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(2, model->rowCount());

    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("a"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("15"));

    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("b"));
    QCOMPARE(model->index(1,1).data().toString(), QLatin1String("'S'"));

    evalExp(QLatin1String("del a; del b"));
}

void TestPython2::testVariableCleanupAfterRestart()
{
    Cantor::DefaultVariableModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    Cantor::Expression* e=evalExp(QLatin1String("a = 15; b = 'S';"));
    QVERIFY(e!=nullptr);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(2, static_cast<QAbstractItemModel*>(model)->rowCount());

    session()->logout();
    session()->login();

    QCOMPARE(0, static_cast<QAbstractItemModel*>(model)->rowCount());
}

void TestPython2::testDictVariable()
{
    Cantor::DefaultVariableModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    Cantor::Expression* e=evalExp(QLatin1String("d = {'value': 33}"));

    QVERIFY(e!=nullptr);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(1, static_cast<QAbstractItemModel*>(model)->rowCount());
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("d"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("{'value': 33}"));

    evalExp(QLatin1String("del d"));
}


QTEST_MAIN(TestPython2)

