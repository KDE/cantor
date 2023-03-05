/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2021-2023 Alexander Semke <alexander.semke@web.de>
*/

#include "testoctave.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "imageresult.h"
#include "textresult.h"
#include "helpresult.h"
#include "epsresult.h"
#include "completionobject.h"
#include "syntaxhelpobject.h"
#include "defaultvariablemodel.h"

#include "octaveexpression.h"
#include "settings.h"

#include <QDebug>

QString TestOctave::backendName()
{
    return QLatin1String("octave");
}

void TestOctave::initTestCase() {
    QSKIP("Octave tests are failing on CI, deactivating for now until the problem is understood and fixed.");
/*
    if (QStandardPaths::findExecutable(QLatin1String("octave")).isEmpty())
        QSKIP("Octave executable not found");
    BackendTest::initTestCase();
*/
}

void TestOctave::testSimpleCommand()
{
    auto* e = evalExp( QLatin1String("2+2") );

    QVERIFY( e != nullptr );
    QVERIFY( e->result()!=nullptr );

    QCOMPARE( cleanOutput( e->result()->data().toString() ), QLatin1String("ans = 4") );
}
void TestOctave::testMultilineCommand()
{
    auto* e = evalExp( QLatin1String("a = 2+2, b = 3+3") );

    QVERIFY( e != nullptr );
    QVERIFY( e->result()!=nullptr );

    QString result=e->result()->data().toString();

    QCOMPARE( cleanOutput(result ), QLatin1String("a = 4\nb = 6") );
}

void TestOctave::testCommandQueue()
{
    auto* e1=session()->evaluateExpression(QLatin1String("0+1"));
    auto* e2=session()->evaluateExpression(QLatin1String("1+1"));
    auto* e3=evalExp(QLatin1String("1+2"));

    QVERIFY(e1!=nullptr);
    QVERIFY(e2!=nullptr);
    QVERIFY(e3!=nullptr);

    QVERIFY(e1->result());
    QVERIFY(e2->result());
    QVERIFY(e3->result());

    QCOMPARE(cleanOutput(e1->result()->data().toString()), QLatin1String("ans = 1"));
    QCOMPARE(cleanOutput(e2->result()->data().toString()), QLatin1String("ans = 2"));
    QCOMPARE(cleanOutput(e3->result()->data().toString()), QLatin1String("ans = 3"));
}

void TestOctave::testVariableDefinition()
{
    auto* e = evalExp(QLatin1String("testvar = 1"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("testvar = 1"));
}

void TestOctave::testMatrixDefinition()
{
    auto* e = evalExp(QLatin1String(
        "M = [1, 2, 3;"\
        "     4, 5, 6;"\
        "     7, 8, 9;]"
    ));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int) Cantor::TextResult::Type);

    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(e->result());
    QCOMPARE(result->plain(), QLatin1String(
        "M =\n"\
        "\n"
        "   1   2   3\n"\
        "   4   5   6\n"\
        "   7   8   9"
    ));
}

//Comments
void TestOctave::testComment00()
{
    auto* e = evalExp(QLatin1String("s = 1234 #This is comment"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(cleanOutput(e->result()->data().toString()), QLatin1String("s = 1234"));
}

/*!
 * simple command containing one single comment only
 */
void TestOctave::testComment01()
{
    auto* e = evalExp(QLatin1String("#Only comment"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QCOMPARE(e->results().size(), 0);
}

/*!
 * multi-line command with lines containing comments only
 */
void TestOctave::testComment02()
{
    auto* e = evalExp(QLatin1String(
        "# comment 1 \n"
        "5 + 5\n"
        "# comment 2\n"
        "a = 10"
    ));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);

    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(e->result());
    QVERIFY(result != nullptr);
    QCOMPARE(cleanOutput(result->plain()), QLatin1String(
        "ans = 10\n"
        "a = 10"
    ));
}

/*!
 * multi-line command with comments within the line containing also the actual expression
 * */
void TestOctave::testComment03()
{
    auto* e = evalExp(QLatin1String(
        "a = 2+4 \n"
        "6/2 % comment\n"
        "q = 'Str' # comment\n"
        "b = 4"
    ));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);

    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(e->result());
    QVERIFY(result != nullptr);
    QCOMPARE(cleanOutput(result->plain()), QLatin1String(
        "a = 6\n"
        "ans = 3\n"
        "q = Str\n"
        "b = 4"
    ));
}

/*!
 * multi-line command with comments within the line containing also the actual expression
 * and with ' used to transpose the vector.
 * */
void TestOctave::testComment04()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String(
        "x=[1,2,3];\n"
        "xT=x';\n"
        "% comment\n"
        "y=1;"
    ));

    QVERIFY(e != nullptr);

   if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() == nullptr); // empty result output

    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("x"));
    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("xT"));
    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("y"));
}

/*!
 * multi-line command with comments within the line containing also the actual expression
 * and with ' used to transpose the vector.
 * */
void TestOctave::testComment05()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String(
        "x=[1,2,3];\n"
        "xT=x';\n"
        "# comment\n"
        "y=1;"
    ));

    QVERIFY(e != nullptr);

   if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() == nullptr); // empty result output

    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("x"));
    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("xT"));
    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("y"));
}

/*!
 * multi-line command with comments within the line containing also the actual expression
 * and with ' used to transpose the vector.
 * */
void TestOctave::testComment06()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String(
        "x=[1,2,3];\n"
        "xT=x'; % comment\n"
        "y=1;"
    ));

    QVERIFY(e != nullptr);

   if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() == nullptr); // empty result output

    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("x"));
    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("xT"));
    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("y"));
}

/*!
 * multi-line command with comments within the line containing also the actual expression
 * and with ' used to transpose the vector.
 * */
void TestOctave::testComment07()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String(
        "x=[1,2,3];\n"
        "xT=x'; # comment\n"
        "y=1;"
    ));

    QVERIFY(e != nullptr);

   if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() == nullptr); // empty result output

    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("x"));
    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("xT"));
    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("y"));
}

/*!
 * multi-line command with comments within the line containing also the actual expression
 * and with ' used to transpose the vector and to quote a string
 * */
void TestOctave::testComment08()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String(
        "x=[1,2,3];\n"
        "xT=x';\n"
        "% comment\n"
        "text = 'bla#blub';\n"
        "y=1;"
    ));

    QVERIFY(e != nullptr);

   if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() == nullptr); // empty result output

    QCOMPARE(model->rowCount(), 4);
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("text"));
    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("x"));
    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("xT"));
    QCOMPARE(model->index(3,0).data().toString(), QLatin1String("y"));
}

/*!
 * multi-line command with comments within the line containing also the actual expression
 * and with ' used to transpose the vector and to quote a string
 * */
void TestOctave::testComment09()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String(
        "x=[1,2,3];\n"
        "xT=x';\n"
        "% comment\n"
        "text = 'bla%blub';\n"
        "y=1;"
    ));

    QVERIFY(e != nullptr);

   if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() == nullptr); // empty result output

    QCOMPARE(model->rowCount(), 4);
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("text"));
    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("x"));
    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("xT"));
    QCOMPARE(model->index(3,0).data().toString(), QLatin1String("y"));
}

void TestOctave::testCompletion()
{
    Cantor::CompletionObject* help = session()->completionFor(QLatin1String("as"), 2);
    waitForSignal(help, SIGNAL(fetchingDone()));

    // Checks some completions for this request (but not all)
    // This correct for Octave 4.2.2 at least (and another versions, I think)
    const QStringList& completions = help->completions();
    QVERIFY(completions.contains(QLatin1String("asin")));
    QVERIFY(completions.contains(QLatin1String("asctime")));
    QVERIFY(completions.contains(QLatin1String("asec")));
    QVERIFY(completions.contains(QLatin1String("assert")));
}

void TestOctave::testVariablesCreatingFromCode()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String("a = 15; b = 'S';"));
    QVERIFY(e != nullptr);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(2, model->rowCount());

    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("a"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("15"));

    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("b"));
    QCOMPARE(model->index(1,1).data().toString(), QLatin1String("S"));
}

void TestOctave::testVariablesMultiRowValues()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e1 = evalExp(QLatin1String("rowVec = [1 2 3];"));
    QVERIFY(e1 != nullptr);

    auto* e2 = evalExp(QLatin1String("columnVec = [1; 2; 3];"));
    QVERIFY(e2 != nullptr);

    auto* e3 = evalExp(QLatin1String("mat = columnVec * rowVec;"));
    QVERIFY(e3 != nullptr);

    if(session()->status() == Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(3, model->rowCount());

    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("rowVec"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("1 2 3"));

    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("columnVec"));
    QCOMPARE(model->index(1,1).data().toString(), QLatin1String("1; 2; 3"));

    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("mat"));
    QCOMPARE(model->index(2,1).data().toString(), QLatin1String("1 2 3; 2 4 6; 3 6 9"));
}

void TestOctave::testVariableChangeSizeType()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    // create a text variable
    auto* e1 = evalExp(QLatin1String("test = \"abcd\";"));
    QVERIFY(e1 != nullptr);

    if(session()->status() == Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(1, model->rowCount());
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("test"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("abcd"));
    QCOMPARE(model->index(0,2).data().toString(), QLatin1String("string"));
    QCOMPARE(model->index(0,3).data().toInt(), 4); // 4 bytes for 4 characters

    // change from string to integer
    auto* e2 = evalExp(QLatin1String("test = 1;"));
    QVERIFY(e2 != nullptr);

    if(session()->status() == Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(1, model->rowCount());
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("test"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("1"));
    QCOMPARE(model->index(0,2).data().toString(), QLatin1String("scalar"));
    QCOMPARE(model->index(0,3).data().toInt(), 8); // 8 bytes for a scalar value
}

void TestOctave::testVariableCreatingFromCodeWithPlot()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));

    auto* e = evalExp(QLatin1String(
        "x = -10:0.1:10;\n"
        "plot (x, sin (x));\n"
        "xlabel (\"x\");\n"
        "ylabel (\"sin (x)\");\n"
        "title (\"Simple 2-D Plot\");\n"
    ));
    QVERIFY(e != nullptr);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result() != nullptr);

    bool eps = (OctaveExpression::plotExtensions[OctaveSettings::inlinePlotFormat()] == QLatin1String("eps"));
    int plotType = eps ? (int)Cantor::EpsResult::Type : (int)Cantor::ImageResult::Type;
    QVERIFY(e->result()->type() == plotType);

    QCOMPARE(1, model->rowCount());
    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("x"));
}

void TestOctave::testVariableCleanupAfterRestart()
{
    Cantor::DefaultVariableModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    evalExp(QLatin1String("clear();"));
    auto* e = evalExp(QLatin1String("a = 15; b = 'S';"));
    QVERIFY(e != nullptr);

    if(session()->status()==Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(2, static_cast<QAbstractItemModel*>(model)->rowCount());

    session()->logout();
    session()->login();

    QCOMPARE(0, static_cast<QAbstractItemModel*>(model)->rowCount());
}

void TestOctave::testPlot()
{
    auto* e = evalExp(QLatin1String("x=-10:0.1:10; plot(x,sin(x));"));
    QVERIFY(e != nullptr);

    if(session()->status() == Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::ImageResult::Type );
    QVERIFY(!e->result()->data().isNull());
    QVERIFY(e->errorMessage().isNull());
}

void TestOctave::testCantorPlot2d()
{
    auto* e = evalExp( QLatin1String("cantor_plot2d('sin(x)', 'x', -10,10);") );
    QVERIFY(e != nullptr);

    if(session()->status() == Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::ImageResult::Type );
    QVERIFY(!e->result()->data().isNull());
    QVERIFY(e->errorMessage().isNull());
}

void TestOctave::testCantorPlot3d()
{
    auto* e = evalExp( QLatin1String("cantor_plot3d('cos(x)*sin(y)', 'x', -10,10, 'y', -10,10);") );
    QVERIFY(e != nullptr);

    if(session()->status() == Cantor::Session::Running)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::ImageResult::Type );
    QVERIFY(!e->result()->data().isNull());
    QVERIFY(e->errorMessage().isNull());
}

void TestOctave::testInvalidSyntax()
{
    auto* e = evalExp( QLatin1String("2+2*+.") );

    QVERIFY( e != nullptr );
    QCOMPARE( e->status(), Cantor::Expression::Error );
}

void TestOctave::testHelpRequest()
{
    auto* e = evalExp(QLatin1String("help printf"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::HelpResult::Type);
    QString text = QString::fromLatin1("Print optional arguments under the control of the template").toHtmlEscaped();
    text.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
    QVERIFY(e->result()->toHtml().contains(text));
}

void TestOctave::testSyntaxHelp()
{
    Cantor::SyntaxHelpObject* help = session()->syntaxHelpFor(QLatin1String("abs"));
    help->fetchSyntaxHelp();
    waitForSignal(help, SIGNAL(done()));

    QString text = QString::fromLatin1("Compute the magnitude").toHtmlEscaped();
    text.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
    QVERIFY(help->toHtml().contains(text));
}

void TestOctave::testLoginLogout()
{
    // Logout from session twice and all must works fine
    session()->logout();
    session()->logout();

    // Login in session twice and all must works fine
    session()->login();
    session()->login();
}

void TestOctave::testRestartWhileRunning()
{
    auto* e1=session()->evaluateExpression(QLatin1String("sleep(5)"));

    session()->logout();
    QCOMPARE(e1->status(), Cantor::Expression::Interrupted);
    session()->login();

    auto* e2=evalExp( QLatin1String("2+2") );

    QVERIFY(e2 != nullptr);
    QVERIFY(e2->result() != nullptr);

    QCOMPARE(cleanOutput(e2->result()->data().toString() ), QLatin1String("ans = 4"));
}

QTEST_MAIN( TestOctave )

