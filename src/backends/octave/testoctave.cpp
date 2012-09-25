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

#include "testoctave.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"
#include "epsresult.h"

#include <kdebug.h>

QString TestOctave::backendName()
{
    return "octave";
}

void TestOctave::testSimpleCommand()
{
    Cantor::Expression* e=evalExp( "2+2" );

    QVERIFY( e!=0 );
    QVERIFY( e->results().size() == 1 );

    QCOMPARE( cleanOutput( e->results().at(0)->toHtml() ), QString("ans =  4") );
}
void TestOctave::testMultilineCommand()
{
    Cantor::Expression* e=evalExp( "a = 2+2, b = 3+3" );

    QVERIFY( e!=0 );
    QVERIFY( e->results().size() == 2 );

    QString result0=e->results().at(0)->toHtml();
    QString result1=e->results().at(1)->toHtml();

    QCOMPARE( cleanOutput(result0 ), QString("a =  4") );
    QCOMPARE( cleanOutput(result1 ), QString("b =  6") );
}

void TestOctave::testPlot()
{
    Cantor::Expression* e=evalExp( "cantor_plot2d('sin(x)', 'x', -10,10);" );

    int cnt=0;
    //give some time to create the image, but at most 5sec
    while(e->results().size()==0)
    {
        QTest::qWait(250);
        cnt+=250;
        if(cnt>5000)
            break;
    }

    QVERIFY( e!=0 );
    QVERIFY( e->results().size() ==0 );

    QCOMPARE( e->results().at(0)->type(), (int)Cantor::EpsResult::Type );
    QVERIFY( !e->results().at(0)->data().isNull() );
}

void TestOctave::testInvalidSyntax()
{
    Cantor::Expression* e=evalExp( "2+2*+." );

    QVERIFY( e!=0 );
    QCOMPARE( e->status(), Cantor::Expression::Error );
}

QTEST_MAIN( TestOctave )

#include "testoctave.moc"
