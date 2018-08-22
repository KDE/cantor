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

#include "testmaxima.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"
#include "epsresult.h"
#include "syntaxhelpobject.h"

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

    QCOMPARE( cleanOutput( e->result()->toHtml() ), QLatin1String("4") );
}

void TestMaxima::testMultilineCommand()
{
    Cantor::Expression* e = evalExp( QLatin1String("2+2;3+3") );

    QVERIFY(e != nullptr);
    QVERIFY(e->results().size() == 2);

    QString result = e->results().at(0)->toHtml();
    QCOMPARE(result, QLatin1String("4"));

    result = e->results().at(1)->toHtml();
    QCOMPARE(result, QLatin1String("6"));
}

//WARNING: for this test to work, Integration of Plots must be anabled
//and CantorLib must be compiled with EPS-support
void TestMaxima::testPlot()
{
    if(QStandardPaths::findExecutable(QLatin1String("gnuplot")).isNull())
    {
        QSKIP("gnuplot not found maxima needs it for plotting", SkipSingle);
    }

    Cantor::Expression* e=evalExp( QLatin1String("plot2d(sin(x), [x, -10,10])") );

    QVERIFY( e!=nullptr );
    QVERIFY( e->result()!=nullptr );

    if(e->result()->type()!= Cantor::EpsResult::Type)
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

    e=evalExp( QString::fromLatin1("%O%1+1" ).arg( id ) );
    QVERIFY( e != nullptr );
    QVERIFY( e->result()!=nullptr );
    QCOMPARE( cleanOutput( e->result()->toHtml() ), QLatin1String( "5" ) );
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

    QCOMPARE(cleanOutput(e1->result()->toHtml()), QLatin1String("1"));
    QCOMPARE(cleanOutput(e2->result()->toHtml()), QLatin1String("2"));
    QCOMPARE(cleanOutput(e3->result()->toHtml()), QLatin1String("3"));
}

void TestMaxima::testSimpleExpressionWithComment()
{
    Cantor::Expression* e=evalExp(QLatin1String("/*this is a comment*/2+2"));
    QVERIFY(e!=nullptr);
    QVERIFY(e->result()!=nullptr);

    QCOMPARE(cleanOutput(e->result()->toHtml()), QLatin1String("4"));
}

void TestMaxima::testCommentExpression()
{
    Cantor::Expression* e=evalExp(QLatin1String("/*this is a comment*/"));
    QVERIFY(e!=nullptr);
    QVERIFY(e->result()==nullptr||e->result()->toHtml().isEmpty());
}

void TestMaxima::testNestedComment()
{
    Cantor::Expression* e=evalExp(QLatin1String("/*/*this is still a comment*/*/2+2/*still/*a*/comment*//**/"));
    QVERIFY(e!=nullptr);
    QVERIFY(e->result()!=nullptr);

    QCOMPARE(cleanOutput(e->result()->toHtml()), QLatin1String("4"));
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
        waitForSignal(session(), SIGNAL( statusChanged(Cantor::Session::Status)));

    //make sure we didn't screw up the session
    Cantor::Expression* e2=evalExp(QLatin1String("2+2"));
    QVERIFY(e2!=nullptr);
    QVERIFY(e2->result()!=nullptr);

    QCOMPARE(cleanOutput(e2->result()->toHtml()), QLatin1String("4"));
}

void TestMaxima::testInformationRequest()
{
    Cantor::Expression* e=session()->evaluateExpression(QLatin1String("integrate(x^n,x)"));
    QVERIFY(e!=nullptr);
    waitForSignal(e, SIGNAL(needsAdditionalInformation(QString)));
    e->addInformation(QLatin1String("N"));

    waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));
    QVERIFY(e->result()!=nullptr);

    QCOMPARE(cleanOutput(e->result()->toHtml()), QLatin1String("x^(n+1)/(n+1)"));
}

void TestMaxima::testSyntaxHelp()
{
    Cantor::SyntaxHelpObject* help = session()->syntaxHelpFor(QLatin1String("simplify_sum"));
    help->fetchSyntaxHelp();
    waitForSignal(help, SIGNAL(done()));

    qWarning()<<help->toHtml();
    QVERIFY(help->toHtml().isEmpty() != false );
}

QTEST_MAIN( TestMaxima )

