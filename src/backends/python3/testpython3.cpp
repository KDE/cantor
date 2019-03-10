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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#include "testpython3.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "imageresult.h"
#include "defaultvariablemodel.h"

QString TestPython3::backendName()
{
    return QLatin1String("python3");
}

void TestPython3::testImportNumpy()
{
    Cantor::Expression* e = evalExp(QLatin1String("import numpy"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
}

void TestPython3::testCodeWithComments()
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

void TestPython3::testPython3Code()
{
    {
    Cantor::Expression* e = evalExp(QLatin1String("print 1 + 2"));

    QVERIFY(e != nullptr);
    QVERIFY(!e->errorMessage().isEmpty());
    }

    {
    Cantor::Expression* e = evalExp(QLatin1String("print(1 + 2)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }
}

void TestPython3::testSimplePlot()
{
    Cantor::Expression* e = evalExp(QLatin1String(
        "import matplotlib\n"
        "import matplotlib.pyplot as plt\n"
        "import numpy as np"
    ));
    QVERIFY(e != nullptr);
    QVERIFY(e->errorMessage().isEmpty() == true);

    //the variable model shouldn't have any entries after the module imports
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);
    QVERIFY(model->rowCount() == 0);

    //create data for plotting
    e = evalExp(QLatin1String(
        "t = np.arange(0.0, 2.0, 0.01)\n"
        "s = 1 + np.sin(2 * np.pi * t)"
    ));
    QVERIFY(e != nullptr);
    QVERIFY(e->errorMessage().isEmpty() == true);

    //the variable model should have two entries now
    QVERIFY(model->rowCount() == 2);

    //plot the data and check the results
    e = evalExp(QLatin1String(
        "plt.plot(t,s)\n"
        "plt.show()"
    ));
    waitForSignal(e, SIGNAL(gotResult()));
    QVERIFY(e != nullptr);
    QVERIFY(e->errorMessage().isEmpty() == true);
    QVERIFY(model->rowCount() == 2); //still only two variables

    //there must be one single image result
    QVERIFY(e->results().size() == 1);
    const Cantor::ImageResult* result = dynamic_cast<const Cantor::ImageResult*>(e->result());
    QVERIFY(result != nullptr);

    evalExp(QLatin1String("del t; del s"));
}

void TestPython3::testVariablesCreatingFromCode()
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

void TestPython3::testVariableCleanupAfterRestart()
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

void TestPython3::testDictVariable()
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

QTEST_MAIN(TestPython3)

