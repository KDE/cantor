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
#include "imageresult.h"
#include "helpresult.h"
#include "syntaxhelpobject.h"
#include "completionobject.h"
#include "defaultvariablemodel.h"

#include <QDebug>
#include <QTextDocument>
#include <QSyntaxHighlighter>

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

    QTest::qWait(100);

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

    QTest::qWait(100);

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
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);
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

void TestR::testSimplePlot()
{
    Cantor::Expression* e = evalExp(QLatin1String("plot(c(1, 4, 9, 16))"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::ImageResult::Type);
}

void TestR::testComplexPlot()
{
    const QLatin1String cmd(
        "# Define 2 vectors\n"
        "cars <- c(1, 3, 6, 4, 9)\n"
        "trucks <- c(2, 5, 4, 5, 12)\n"
        "\n"
        "# Calculate range from 0 to max value of cars and trucks\n"
        "g_range <- range(0, cars, trucks)\n"
        "\n"
        "# Graph autos using y axis that ranges from 0 to max \n"
        "# value in cars or trucks vector.  Turn off axes and \n"
        "# annotations (axis labels) so we can specify them ourself\n"
        "plot(cars, type=\"o\", col=\"blue\", ylim=g_range, \n"
        "   axes=FALSE, ann=FALSE)\n"
        "\n"
        "# Make x axis using Mon-Fri labels\n"
        "axis(1, at=1:5, lab=c(\"Mon\",\"Tue\",\"Wed\",\"Thu\",\"Fri\"))\n"
        "\n"
        "# Make y axis with horizontal labels that display ticks at \n"
        "# every 4 marks. 4*0:g_range[2] is equivalent to c(0,4,8,12).\n"
        "axis(2, las=1, at=4*0:g_range[2])\n"
        "\n"
        "# Create box around plot\n"
        "box()\n"
        "\n"
        "# Graph trucks with red dashed line and square points\n"
        "lines(trucks, type=\"o\", pch=22, lty=2, col=\"red\")\n"
        "\n"
        "# Create a title with a red, bold/italic font\n"
        "title(main=\"Autos\", col.main=\"red\", font.main=4)\n"
        "\n"
        "# Label the x and y axes with dark green text\n"
        "title(xlab=\"Days\", col.lab=rgb(0,0.5,0))\n"
        "title(ylab=\"Total\", col.lab=rgb(0,0.5,0))\n"
        "\n"
        "# Create a legend at (1, g_range[2]) that is slightly smaller \n"
        "# (cex) and uses the same line colors and points used by \n"
        "# the actual plots \n"
        "legend(1, g_range[2], c(\"cars\",\"trucks\"), cex=0.8, \n"
        "   col=c(\"blue\",\"red\"), pch=21:22, lty=1:2);\n"
    );

    Cantor::Expression* e = evalExp(cmd);

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::ImageResult::Type);
    evalExp(QLatin1String("rm(cars, trucks, g_range)"));
}

void TestR::testHelpRequest()
{
    QSKIP("Skip, until moving R help to help panel insteadof scripteditor");
    Cantor::Expression* e = evalExp(QLatin1String("?print"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::HelpResult::Type);
}

void TestR::testSyntaxHelp()
{
    QSKIP("Skip, until adding this feature to R backend");
    Cantor::SyntaxHelpObject* help = session()->syntaxHelpFor(QLatin1String("filter"));
    help->fetchSyntaxHelp();
    waitForSignal(help, SIGNAL(done()));

    QVERIFY(help->toHtml().contains(QLatin1String("filter")));
}

QTEST_MAIN( TestR )

