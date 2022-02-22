/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 by Alexander Semke (alexander.semke@web.de)
*/

#ifndef _TESTMAXIMA_H
#define _TESTMAXIMA_H

#include "backendtest.h"

/** This class test some of the basic functions of the maxima backend
    The different tests represent some general expressions, as well
    as expressions, that are known to have caused problems in earlier
    versions
**/
class TestMaxima : public BackendTest
{
  Q_OBJECT

private Q_SLOTS:
    //tests evaluating a simple command
    void testSimpleCommand();
    //tests a command, containing more than 1 line
    void testMultilineCommand();
    //tests if the command queue works correctly
    void testCommandQueue();
    //tests doing a plot
    void testPlot();
    void testPlotWithAnotherTextResults();

    /* errors and warnings */

    //tests a syntax error (not closing bracket)
    void testInvalidSyntax();
    void testWarning01();
    void testWarning02();
    //tests if the expression numbering works
    void testExprNumbering();
    void testInvalidAssignment();

    /* comments */
    void testSimpleExpressionWithComment();
    void testCommentExpression();
    void testNestedComment();
    void testUnmatchedComment();

    /* tests where additional input is required */
    void testInformationRequest();
    void testHelpRequest();
    void testSyntaxHelp();

    void testCompletion();

    void testVariableModel();

    void testLispMode01();

    void testLoginLogout();
    void testRestartWhileRunning();

private:
    QString backendName() override;
};

#endif /* _TESTMAXIMA_H */
