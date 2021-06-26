/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#include "testpython.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "imageresult.h"
#include "defaultvariablemodel.h"
#include "completionobject.h"

#include "settings.h"

QString TestPython3::backendName()
{
    return QLatin1String("python");
}

void TestPython3::testSimpleCommand()
{
    Cantor::Expression* e = evalExp(QLatin1String("2+2"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result());
    QVERIFY(e->result()->data().toString() == QLatin1String("4"));
}

void TestPython3::testMultilineCommand()
{
    Cantor::Expression* e = evalExp(QLatin1String("print(2+2)\nprint(7*5)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result());
    QVERIFY(e->result()->data().toString() == QLatin1String("4\n35"));
}

void TestPython3::testCommandQueue()
{
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

void TestPython3::testCommentExpression()
{
    Cantor::Expression* e = evalExp(QLatin1String("#only comment"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QCOMPARE(e->results().size(), 0);
}

void TestPython3::testSimpleExpressionWithComment()
{
    Cantor::Expression* e = evalExp(QLatin1String("2+2 # comment"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result());
    QVERIFY(e->result()->data().toString() == QLatin1String("4"));
}

void TestPython3::testMultilineCommandWithComment()
{
    Cantor::Expression* e = evalExp(QLatin1String(
        "print(2+2) \n"
        "#comment in middle \n"
        "print(7*5)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result());
    QVERIFY(e->result()->data().toString() == QLatin1String("4\n35"));
}

void TestPython3::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2*+.") );

    QVERIFY( e!=nullptr );
    QCOMPARE( e->status(), Cantor::Expression::Error );
}

void TestPython3::testCompletion()
{
    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    Cantor::CompletionObject* help = session()->completionFor(QLatin1String("p"), 1);
    waitForSignal(help, SIGNAL(fetchingDone()));

    // Checks all completions for this request
    const QStringList& completions = help->completions();
    QCOMPARE(completions.size(), 4);
    QVERIFY(completions.contains(QLatin1String("pass")));
    QVERIFY(completions.contains(QLatin1String("pow")));
    QVERIFY(completions.contains(QLatin1String("print")));
    QVERIFY(completions.contains(QLatin1String("property")));
}


void TestPython3::testImportStatement()
{
    Cantor::Expression* e = evalExp(QLatin1String("import sys"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
}

void TestPython3::testCodeWithComments()
{
    {
    Cantor::Expression* e = evalExp(QLatin1String("#comment\n1+2"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result());
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }

    {
    Cantor::Expression* e = evalExp(QLatin1String("     #comment\n1+2"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result());
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
    QVERIFY(e->result());
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }
}

void TestPython3::testSimplePlot()
{
    if (!PythonSettings::integratePlots())
        QSKIP("This test needs enabled plots integration in Python3 settings", SkipSingle);

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

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));
    //the variable model should have two entries now
    QVERIFY(model->rowCount() == 2);

    //plot the data and check the results
    e = evalExp(QLatin1String(
        "plt.plot(t,s)\n"
        "plt.show()"
    ));

    QVERIFY(e != nullptr);
    if (e->result() == nullptr)
        waitForSignal(e, SIGNAL(gotResult()));

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
    if (!PythonSettings::variableManagement())
        QSKIP("This test needs enabled variable management in Python3 settings", SkipSingle);

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
    if (!PythonSettings::variableManagement())
        QSKIP("This test needs enabled variable management in Python3 settings", SkipSingle);

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

void TestPython3::testInterrupt()
{
    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String("import time; time.sleep(150)"));
    Cantor::Expression* e2=session()->evaluateExpression(QLatin1String("2"));

    if (e1->status() != Cantor::Expression::Queued)
        waitForSignal(e1, SIGNAL(statusChanged(Cantor::Expression::Status)));

    if (e1->status() != Cantor::Expression::Computing)
        waitForSignal(e1, SIGNAL(statusChanged(Cantor::Expression::Status)));

    if (e2->status() != Cantor::Expression::Queued)
        waitForSignal(e2, SIGNAL(statusChanged(Cantor::Expression::Status)));

    // Without this delay, server don't interrupt even if got interrupt signal (via OS kill)
    // Also, if the server won't interrupt, the test will fail without reasonable reason
    QTest::qWait(100);

    QCOMPARE(e1->status(), Cantor::Expression::Computing);
    QCOMPARE(e2->status(), Cantor::Expression::Queued);

    while(session()->status() != Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    session()->interrupt();

    while(session()->status() != Cantor::Session::Done)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e1->status(), Cantor::Expression::Interrupted);
    QCOMPARE(e2->status(), Cantor::Expression::Interrupted);

    Cantor::Expression* e = evalExp(QLatin1String("2+2"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result());
    QCOMPARE(e->result()->data().toString(), QLatin1String("4"));
}

void TestPython3::testWarning()
{
    Cantor::Expression* e = evalExp(QLatin1String("import warnings; warnings.warn('Test')"));

    QVERIFY(e != nullptr);

    while(session()->status() != Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QCOMPARE(e->results().size(), 1);
}

QTEST_MAIN(TestPython3)

