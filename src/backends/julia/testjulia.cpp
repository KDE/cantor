/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "testjulia.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "textresult.h"
#include "helpresult.h"
#include "imageresult.h"
#include "defaultvariablemodel.h"
#include "completionobject.h"

#include "settings.h"

QString TestJulia::backendName()
{
    return QLatin1String("julia");
}

void TestJulia::testOneLine()
{
    Cantor::Expression *e = evalExp(QLatin1String("2 + 3"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("5"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testOneLineWithPrint()
{
    Cantor::Expression *e = evalExp(QLatin1String("print(2 + 3)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("5"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testException()
{
    Cantor::Expression *e = evalExp(QLatin1String("sqrt(-1)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result());
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QVERIFY(
        !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument."
        ))
    );
}

void TestJulia::testSyntaxError()
{
    Cantor::Expression *e = evalExp(QLatin1String("for i = 1:10\nprint(i)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QVERIFY(
        !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "syntax: incomplete: \"for\" at none:1 requires end"
        ))
    );
}

void TestJulia::testMultilineCode()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "q = 0; # comment\n"
        "# sdfsdf\n"
        "for i = 1:10\n"
        "    global q\n"
        "    q += i\n"
        "end\n"
        "print(q)"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("55"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testPartialResultOnException()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "for i = 1:5\n"
        "    print(i)\n"
        "end\n"
        "sqrt(-1)\n"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("12345"));
    QVERIFY(
        !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument."
        ))
    );
}

void TestJulia::testInlinePlot()
{
    if (!JuliaSettings::integratePlots())
        QSKIP("This test needs enabled plots integration in Julia settings", SkipSingle);

    Cantor::Expression *e = evalExp(QLatin1String(
        "import GR\n"
        "GR.plot([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1], sin)"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::ImageResult::Type);
}

void TestJulia::testInlinePlotWithExceptionAndPartialResult()
{
    if (!JuliaSettings::integratePlots())
        QSKIP("This test needs enabled plots integration in Julia settings", SkipSingle);

    Cantor::Expression *e = evalExp(QLatin1String(
        "import GR\n"
        "print(\"gonna plot\")\n"
        "sqrt(-1)\n"
        "GR.plot([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1], sin)\n"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("gonna plot"));
    QVERIFY(
         !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument."
        ))
    );
}

void TestJulia::testAddVariablesFromCode()
{
    evalExp(QLatin1String("a = 0; b = 1; c = 2; d = 3\n"));

    auto variableModel = session()->variableModel();
    QStringList variableNames =
    QString::fromLatin1("a b c d").split(QLatin1String(" "));

    for (int i = 0; i < variableNames.size(); i++) {
        QModelIndexList matchedVariables = variableModel->match(
            variableModel->index(0, 0),
            Qt::DisplayRole,
            QVariant::fromValue(variableNames[i]),
            -1,
            Qt::MatchExactly
        );
        QCOMPARE(matchedVariables.size(), 1);
        auto match = matchedVariables[0];
        auto valueIndex =
            variableModel->index(match.row(), match.column() + 1);
        QVERIFY(
            valueIndex.data(Qt::DisplayRole) ==
            QVariant::fromValue(QString::number(i))
        );
    }
}

void TestJulia::testAddVariablesFromManager()
{
    Cantor::DefaultVariableModel* variableModel = session()->variableModel();
    QStringList variableNames =
    QString::fromLatin1("a b c d").split(QLatin1String(" "));

    for (int i = 0; i < variableNames.size(); i++) {
        variableModel->addVariable(variableNames[i], QString::number(i));

        QModelIndexList matchedVariables = variableModel->match(
            variableModel->index(0, 0),
            Qt::DisplayRole,
            QVariant::fromValue(variableNames[i]),
            -1,
            Qt::MatchExactly
        );
        QCOMPARE(matchedVariables.size(), 1);
        auto match = matchedVariables[0];
        auto valueIndex =
            variableModel->index(match.row(), match.column() + 1);
        QVERIFY(
            valueIndex.data(Qt::DisplayRole) ==
            QVariant::fromValue(QString::number(i))
        );
    }
}

void TestJulia::testRemoveVariables()
{
    Cantor::DefaultVariableModel* variableModel = session()->variableModel();
    QStringList variableNames =
    QString::fromLatin1("a b c d").split(QLatin1String(" "));

    for (int i = 0; i < variableNames.size(); i++) {
        variableModel->addVariable(variableNames[i], QString::number(i));
    }
    for (int i = 0; i < variableNames.size(); i += 2) {
        variableModel->removeVariable(variableNames[i]);
    }

    for (int i = 0; i < variableNames.size(); i++) {
        QModelIndexList matchedVariables = variableModel->match(
            variableModel->index(0, 0),
            Qt::DisplayRole,
            QVariant::fromValue(variableNames[i]),
            -1,
            Qt::MatchExactly
        );
        if (i % 2 == 0) {
            QVERIFY(matchedVariables.isEmpty());
        } else {
            QCOMPARE(matchedVariables.size(), 1);
            auto match = matchedVariables[0];
            auto valueIndex =
                variableModel->index(match.row(), match.column() + 1);
            QVERIFY(
                valueIndex.data(Qt::DisplayRole) ==
                QVariant::fromValue(QString::number(i))
            );
        }
    }
}

void TestJulia::testAutoCompletion()
{
    auto prefix = QLatin1String("ex");
    auto completionObject = session()->completionFor(prefix);
    waitForSignal(completionObject, SIGNAL(fetchingDone()));
    auto completions = completionObject->completions();

    QStringList completionsToCheck;
    completionsToCheck << QLatin1String("exit");
    completionsToCheck << QLatin1String("exponent");
    completionsToCheck << QLatin1String("exp");

    for (auto completion : completionsToCheck) {
        QVERIFY(completions.contains(completion));
    }

    for (auto completion : completions) {
        QVERIFY(completion.startsWith(prefix));
    }
}

void TestJulia::testComplexAutocompletion()
{
    auto prefix = QLatin1String("Base.Ma");
    auto completionObject = session()->completionFor(prefix);
    waitForSignal(completionObject, SIGNAL(fetchingDone()));
    auto completions = completionObject->completions();

    QStringList completionsToCheck;
    completionsToCheck << QLatin1String("Base.MainInclude");
    completionsToCheck << QLatin1String("Base.Math");
    completionsToCheck << QLatin1String("Base.Matrix");

    for (auto completion : completionsToCheck) {
        QVERIFY(completions.contains(completion));
    }

    for (auto completion : completions) {
        QVERIFY(completion.startsWith(prefix));
    }
}

void TestJulia::testExpressionQueue()
{
    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String("0+1"));
    Cantor::Expression* e2=session()->evaluateExpression(QLatin1String("1+1"));
    QVERIFY(session()->status() == Cantor::Session::Running);
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

void TestJulia::testHelpRequest()
{
    QSKIP("Skip, until we add this functionality to Julia backend", SkipSingle);
    Cantor::Expression* e = evalExp(QLatin1String("?print"));

    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Status::Done);
    QVERIFY(e->result() != nullptr);
    QCOMPARE(e->result()->type(), (int)Cantor::HelpResult::Type);
    QString text = QString::fromLatin1("Write to io (or to the default output stream stdout").toHtmlEscaped();
    text.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
    QVERIFY(e->result()->toHtml().contains(text));
}

void TestJulia::testLoginLogout()
{
    // Logout from session twice and all must works fine
    session()->logout();
    session()->logout();

    // Login in session twice and all must works fine
    session()->login();
    session()->login();
}

void TestJulia::testRestartWhileRunning()
{
    Cantor::Expression* e1=session()->evaluateExpression(QLatin1String("sleep(5)"));

    session()->logout();
    QCOMPARE(session()->status(), Cantor::Session::Disable);
    QCOMPARE(e1->status(), Cantor::Expression::Interrupted);
    session()->login();

    Cantor::Expression* e2=evalExp( QLatin1String("2+2") );

    QVERIFY(e2 != nullptr);
    QVERIFY(e2->result() != nullptr);

    QCOMPARE(cleanOutput(e2->result()->data().toString() ), QLatin1String("4"));
}

QTEST_MAIN(TestJulia)

