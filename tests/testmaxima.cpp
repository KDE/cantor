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

//simple method that removes whitespaces/other irrelevant stuff,
//so comparing results is easier
QString cleanOutput( const QString& out )
{
    QString cleaned=out;
    cleaned.replace( "&nbsp;"," " );
    cleaned.remove( "<br/>" );
    cleaned.replace( QChar::ParagraphSeparator, '\n' );
    cleaned.replace( QRegExp( "\\n\\n" ), "\n" );
    cleaned.replace( QRegExp( "\\n\\s*" ), "\n" );

    return cleaned.trimmed();
}

TestMaxima::TestMaxima()
{
    m_session=createSession();
}

Cantor::Session* TestMaxima::createSession()
{
    Cantor::Backend* b=Cantor::Backend::createBackend( "maxima" );
    if(!b )
        return 0;

    Cantor::Session* session=b->createSession();
    session->login();

    QEventLoop loop;
    connect( session, SIGNAL( ready() ), &loop, SLOT( quit() ) );
    loop.exec();

   return session;
}

Cantor::Expression* TestMaxima::evalExp(const QString& exp )
{
   Cantor::Expression* e=m_session->evaluateExpression(exp);

   //Create a timeout, that kills the eventloop, if the expression doesn't finish
   QPointer<QTimer> timeout=new QTimer( this );
   timeout->setSingleShot( true );
   timeout->start( 6000 );
   QEventLoop loop;
   connect( timeout, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
   connect( e, SIGNAL( statusChanged( Cantor::Expression::Status ) ), &loop, SLOT( quit() ) );

   loop.exec();

   delete timeout;

   return e;
}

void TestMaxima::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( "2+2" );

    QVERIFY( e!=0 );
    QVERIFY( e->result()!=0 );

    QCOMPARE( cleanOutput( e->result()->toHtml() ), QString("4") );
}
void TestMaxima::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( "2+2;3+3" );

    QVERIFY( e!=0 );
    QVERIFY( e->result()!=0 );

    QString result=e->result()->toHtml();

    QCOMPARE( cleanOutput(result ), QString("4\n6") );
}

//WARNING: for this test to work, Integration of Plots must be anabled
//and CantorLib must be compiled with EPS-support
void TestMaxima::testPlot()
{
    Cantor::Expression* e=evalExp( "plot2d(sin(x), [x, -10,10])" );

    QVERIFY( e!=0 );
    QVERIFY( e->result()!=0 );

    QVERIFY( e->result()->type()==Cantor::EpsResult::Type );
    QVERIFY( !e->result()->data().isNull() );
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
    QVERIFY( e->result()!=0 );
    QCOMPARE( cleanOutput( e->result()->toHtml() ), QString( "5" ) );
}

QTEST_MAIN( TestMaxima )

#include "testmaxima.moc"
