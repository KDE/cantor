/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "testmaxima.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "textresult.h"
#include "imageresult.h"
#include "epsresult.h"
#include "syntaxhelpobject.h"
#include "completionobject.h"
#include "defaultvariablemodel.h"

#include <config-cantorlib.h>

#include <QDebug>

QString TestMaxima::backendName()
{
    return QLatin1String("maxima");
}


void TestMaxima::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput( e->result()->data().toString() ), QLatin1String("4") );
}

void TestMaxima::testMultilineCommand()
{
    Cantor::Expression* e = evalExp( QLatin1String("2+2;3+3") );

    QVERIFY(e != nullptr);
    QVERIFY(e->results().size() == 2);

    QCOMPARE(e->results().at(0)->data().toString(), QLatin1String("4"));
    QCOMPARE(e->results().at(1)->data().toString(), QLatin1String("6"));
}

//WARNING: for this test to work, Integration of Plots must be enabled
//and CantorLib must be compiled with EPS-support
void TestMaxima::testPlot()
{
    if(QStandardPaths::findExecutable(QLatin1String("gnuplot")).isNull())
    {
        QSKIP("gnuplot not found, maxima needs it for plotting", SkipSingle);
    }

    Cantor::Expression* e=evalExp( QLatin1String("plot2d(sin(x), [x, -10,10])") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    if(!e->result())
    {
        waitForSignal(e, SIGNAL(gotResult()));
    }

#ifdef WITH_EPS
    QCOMPARE( e->result()->type(), (int)Cantor::EpsResult::Type );
#else
    QCOMPARE( e->result()->type(), (int)Cantor::ImageResult::Type );
#endif
    QVERIFY( !e->result()->data().isNull() );
    QVERIFY( e->errorMessage().isNull() );
}

void TestMaxima::testPlotWithAnotherTextResults()
{
    if(QStandardPaths::findExecutable(QLatin1String("gnuplot")).isNull())
    {
        QSKIP("gnuplot not found, maxima needs it for plotting", SkipSingle);
    }

    Cantor::Expression* e=evalExp( QLatin1String(
        "2*2; \n"
        "plot2d(sin(x), [x, -10,10]); \n"
        "4*4;"
    ));

    if (e->results().at(1)->type() == Cantor::TextResult::Type)
        waitForSignal(e, SIGNAL(resultReplaced));

    QVERIFY( e!=nullptr );
    QVERIFY( e->errorMessage().isNull() );
    QCOMPARE(e->results().size(), 3);

    QCOMPARE(e->results().at(0)->data().toString(), QLatin1String("4"));

#ifdef WITH_EPS
    QCOMPARE( e->results().at(1)->type(), (int)Cantor::EpsResult::Type );
#else
    QCOMPARE( e->results().at(1)->type(), (int)Cantor::ImageResult::Type );
#endif
    QVERIFY( !e->results().at(1)->data().isNull() );

    QCOMPARE(e->results().at(2)->data().toString(), QLatin1String("16"));
}

void TestMaxima::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( QLatin1String("2+2*(") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->status()==Cantor::Expression::Error );
}

void TestMaxima::testExprNumbering()
{
    Cantor::Expression* e=evalExp( QLatin1String("kill(labels)") ); //first reset the labels

    e=evalExp( QLatin1String("2+2") );
    QVERIFY( e!=nullptr );
    int id=e->id();
    QCOMPARE( id, 1 );

    e=evalExp( QString::fromLatin1("%o%1+1" ).arg( id ) );
    QVERIFY( e != nullptr );
    QVERIFY( e->result()!=nullptr );
    QCOMPARE( cleanOutput( e->result()->data().toString() ), QLatin1String( "5" ) );
}

void TestMaxima::testCommandQueue()
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

void TestMaxima::testSimpleExpressionWithComment()
{
    Cantor::Expression* e=evalExp(QLatin1String("/*this is a comment*/2+2"));
    QVERIFY(e!=nullptr);
    QVERIFY(e->result()!=nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("4"));
}

void TestMaxima::testCommentExpression()
{
    Cantor::Expression* e=evalExp(QLatin1String("/*this is a comment*/"));
    QVERIFY(e!=nullptr);
    QVERIFY(e->result()==nullptr||e->result()->data().toString().isEmpty());
}

void TestMaxima::testNestedComment()
{
    Cantor::Expression* e=evalExp(QLatin1String("/*/*this is still a comment*/*/2+2/*still/*a*/comment*//**/"));
    QVERIFY(e!=nullptr);
    QVERIFY(e->result()!=nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("4"));
}

void TestMaxima::testUnmatchedComment()
{
    Cantor::Expression* e=evalExp(QLatin1String("/*this comment doesn't end here!"));
    QVERIFY(e!=nullptr);
    QVERIFY(e->result()==nullptr);
    QVERIFY(e->status()==Cantor::Expression::Error);
}

void TestMaxima::testInvalidAssignment()
{
    Cantor::Expression* e=evalExp(QLatin1String("0:a"));
    QVERIFY(e!=nullptr);
    //QVERIFY(e->result()==0);
    //QVERIFY(e->status()==Cantor::Expression::Error);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    //make sure we didn't screw up the session
    Cantor::Expression* e2=evalExp(QLatin1String("2+2"));
    QVERIFY(e2!=nullptr);
    QVERIFY(e2->result()!=nullptr);

    QCOMPARE(cleanOutput(e2->result()->data().toString()), QLatin1String("4"));
}

void TestMaxima::testInformationRequest()
{
    Cantor::Expression* e=session()->evaluateExpression(QLatin1String("integrate(x^n,x)"));
    QVERIFY(e!=nullptr);
    waitForSignal(e, SIGNAL(needsAdditionalInformation(QString)));
    e->addInformation(QLatin1String("N"));

    waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));
    QVERIFY(e->result()!=nullptr);

    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("x^(n+1)/(n+1)"));
}

void TestMaxima::testSyntaxHelp()
{
    Cantor::SyntaxHelpObject* help = session()->syntaxHelpFor(QLatin1String("simplify_sum"));
    help->fetchSyntaxHelp();
    waitForSignal(help, SIGNAL(done()));

    bool trueHelpMessage= help->toHtml().contains(QLatin1String("simplify_sum"));
    bool problemsWithMaximaDocs = help->toHtml().contains(QLatin1String("INTERNAL-SIMPLE-FILE-ERROR"));
    QVERIFY(trueHelpMessage || problemsWithMaximaDocs);
}

void TestMaxima::testCompletion()
{
    Cantor::CompletionObject* help = session()->completionFor(QLatin1String("ask"), 3);
    waitForSignal(help, SIGNAL(fetchingDone()));

    // Checks all completions for this request
    // This correct for Maxima 5.41.0
    const QStringList& completions = help->completions();
    QVERIFY(completions.contains(QLatin1String("asksign")));
    QVERIFY(completions.contains(QLatin1String("askinteger")));
    QVERIFY(completions.contains(QLatin1String("askexp")));
}

void TestMaxima::testHelpRequest()
{
    //execute "??print"
    Cantor::Expression* e = session()->evaluateExpression(QLatin1String("??print"));
    QVERIFY(e != nullptr);

    //help result will be shown, but maxima still expects further input
    waitForSignal(e, SIGNAL(needsAdditionalInformation(QString)));
    QVERIFY(e->status() != Cantor::Expression::Done);

    //ask for help for the first flag of the print command
    e->addInformation(QLatin1String("0"));

    //no further input is required, we're done
    waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));
    if (e->status() == Cantor::Expression::Computing)
        waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));
    QVERIFY(e->status() == Cantor::Expression::Done);
}

void TestMaxima::testVariableModel()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    Cantor::Expression* e1=evalExp(QLatin1String("a: 15"));
    Cantor::Expression* e2=evalExp(QLatin1String("a: 15; b: \"Hello, world!\""));
    Cantor::Expression* e3=evalExp(QLatin1String("l: [1,2,3]"));
    QVERIFY(e1!=nullptr);
    QVERIFY(e2!=nullptr);
    QVERIFY(e3!=nullptr);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(3, model->rowCount());

    QVariant name = model->index(0,0).data();
    QCOMPARE(name.toString(),QLatin1String("a"));

    QVariant value = model->index(0,1).data();
    QCOMPARE(value.toString(),QLatin1String("15"));

    QVariant name1 = model->index(1,0).data();
    QCOMPARE(name1.toString(),QLatin1String("b"));

    QVariant value1 = model->index(1,1).data();
    QCOMPARE(value1.toString(),QLatin1String("\"Hello, world!\""));

    QVariant name2 = model->index(2,0).data();
    QCOMPARE(name2.toString(),QLatin1String("l"));

    QVariant value2 = model->index(2,1).data();
    QCOMPARE(value2.toString(),QLatin1String("[1,2,3]"));
}

void TestMaxima::testLoginLogout()
{
    // Logout from session twice and all must works fine
    session()->logout();
    session()->logout();

    // Login in session twice and all must works fine
    session()->login();
    session()->login();
}

void TestMaxima::testRestartWhileRunning()
{
    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String(":lisp (sleep 5)"));

    session()->logout();
    QCOMPARE(e1->status(), Cantor::Expression::Interrupted);
    session()->login();

    Cantor::Expression* e2=evalExp( QLatin1String("2+2") );

    QVERIFY(e2 != nullptr);
    QVERIFY(e2->result() != nullptr);

    QCOMPARE(cleanOutput(e2->result()->data().toString() ), QLatin1String("4"));
}

QTEST_MAIN( TestMaxima )

