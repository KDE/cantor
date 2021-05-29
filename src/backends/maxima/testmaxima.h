/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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

    //tests a syntax error (not closing bracket)
    void testInvalidSyntax();
    //tests if the expression numbering works
    void testExprNumbering();

    //some tests to see if comments are working correctly
    void testSimpleExpressionWithComment();
    void testCommentExpression();
    void testNestedComment();
    void testUnmatchedComment();

    void testInvalidAssignment();

    void testInformationRequest();
    void testHelpRequest();
    void testSyntaxHelp();
    void testCompletion();

    void testVariableModel();

    void testLoginLogout();
    void testRestartWhileRunning();
private:
    QString backendName() override;
};

#endif /* _TESTMAXIMA_H */
