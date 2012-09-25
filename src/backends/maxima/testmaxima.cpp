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

#include <kdebug.h>

QString TestMaxima::backendName()
{
    return "maxima";
}


void TestMaxima::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( "2+2" );

    QVERIFY( e!=0 );
    QVERIFY( e->results().size() == 1 );

    QCOMPARE( cleanOutput( e->results().at(0)->toHtml() ), QString("4") );
}

void TestMaxima::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( "2+2;3+3" );

    QVERIFY( e!=0 );
    QVERIFY( e->results().size() == 2 );

    QString result0=e->results().at(0)->toHtml();
    QString result1=e->results().at(1)->toHtml();

    QCOMPARE( cleanOutput(result0), QString("4") );
    QCOMPARE( cleanOutput(result1), QString("6") );
}

//WARNING: for this test to work, Integration of Plots must be anabled
//and CantorLib must be compiled with EPS-support
void TestMaxima::testPlot()
{
    Cantor::Expression* e=evalExp( "plot2d(sin(x), [x, -10,10])" );

    QVERIFY( e!=0 );
    QVERIFY( e->results().size() ==0 );

    Cantor::Result* result = e->results().at(0);
    if(result->type()!= Cantor::EpsResult::Type)
    {
        waitForSignal(e, SIGNAL(gotResult()));
    }

    QCOMPARE( result->type(), (int)Cantor::EpsResult::Type );
    QVERIFY( !result->data().isNull() );
    QVERIFY( e->errorMessage().isNull() );
}

void TestMaxima::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( "2+2*(" );

    QVERIFY( e!=0 );
    QVERIFY( e->status()==Cantor::Expression::Error );
}

void TestMaxima::testExprNumbering()
{
    Cantor::Expression* e=evalExp( "kill(labels)" ); //first reset the labels

    e=evalExp( "2+2" );
    QVERIFY( e!=0 );
    int id=e->id();
    QCOMPARE( id, 1 );

    e=evalExp( QString("%O%1+1" ).arg( id ) );
    QVERIFY( e != 0 );
    QVERIFY( e->results().size() == 1 );
    QCOMPARE( cleanOutput( e->results().at(0)->toHtml() ), QString( "5" ) );
}

void TestMaxima::testCommandQueue()
{
    //only wait for the last Expression to return, so the queue gets
    //actually filled

    Cantor::Expression* e1=session()->evaluateExpression("0+1");
    Cantor::Expression* e2=session()->evaluateExpression("1+1");
    Cantor::Expression* e3=evalExp("1+2");

    QVERIFY(e1!=0);
    QVERIFY(e2!=0);
    QVERIFY(e3!=0);

    QVERIFY(e1->results().size() == 1);
    QVERIFY(e2->results().size() == 1);
    QVERIFY(e3->results().size() == 1);

    QCOMPARE(cleanOutput(e1->results().at(0)->toHtml()), QString("1"));
    QCOMPARE(cleanOutput(e2->results().at(0)->toHtml()), QString("2"));
    QCOMPARE(cleanOutput(e3->results().at(0)->toHtml()), QString("3"));
}

void TestMaxima::testSimpleExpressionWithComment()
{
    Cantor::Expression* e=evalExp("/*this is a comment*/2+2");
    QVERIFY(e!=0);
    QVERIFY(e->results().size() == 1);

    QCOMPARE(cleanOutput(e->results().at(0)->toHtml()), QString("4"));
}

void TestMaxima::testCommentExpression()
{
    Cantor::Expression* e=evalExp("/*this is a comment*");
    QVERIFY(e!=0);
    QVERIFY(e->results().size()==0||e->results().at(0)->toHtml().isEmpty());
}

void TestMaxima::testNestedComment()
{
    Cantor::Expression* e=evalExp("/*/*this is still a comment*/*/2+2/*still/*a*/comment*//**/");
    QVERIFY(e!=0);
    QVERIFY(e->results().size() == 1);

    QCOMPARE(cleanOutput(e->results().at(0)->toHtml()), QString("4"));
}

void TestMaxima::testUnmatchedComment()
{
    Cantor::Expression* e=evalExp("/*this comment doesn't end here!");
    QVERIFY(e!=0);
    QVERIFY(e->results().size()==0);
    QVERIFY(e->status()==Cantor::Expression::Error);
}

void TestMaxima::testInvalidAssignment()
{
    Cantor::Expression* e=evalExp("0:a");
    QVERIFY(e!=0);
    //QVERIFY(e->result()==0);
    //QVERIFY(e->status()==Cantor::Expression::Error);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL( statusChanged(Cantor::Session::Status)));

    //make sure we didn't screw up the session
    Cantor::Expression* e2=evalExp("2+2");
    QVERIFY(e2!=0);
    QVERIFY(e2->results().size() == 1);

    QCOMPARE(cleanOutput(e2->results().at(0)->toHtml()), QString("4"));
}

void TestMaxima::testInformationRequest()
{
    Cantor::Expression* e=session()->evaluateExpression("integrate(x^n,x)");
    QVERIFY(e!=0);
    waitForSignal(e, SIGNAL(needsAdditionalInformation(QString)));
    e->addInformation("nonzero;");

    waitForSignal(e, SIGNAL(statusChanged(Cantor::Expression::Status)));
    QVERIFY(e->results().size() == 1);

    QCOMPARE(cleanOutput(e->results().at(0)->toHtml()), QString("x^(n+1)/(n+1)"));
}


QTEST_MAIN( TestMaxima )

#include "testmaxima.moc"
