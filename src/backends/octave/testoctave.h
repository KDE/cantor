/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _TESTOCTAVE_H
#define _TESTOCTAVE_H

#include "backendtest.h"

/** This class test some of the basic functions of the Octave backend
    The different tests represent some general expressions, as well
    as expressions, that are known to have caused problems in earlier
    versions
**/
class TestOctave : public BackendTest
{
  Q_OBJECT

private Q_SLOTS:
    //tests evaluating a simple command
    void testSimpleCommand();
    //tests a command, containing more than 1 line
    void testMultilineCommand();
    //tests if the command queue works correctly
    void testCommandQueue();

    void testVariableDefinition();
    void testMatrixDefinition();

    //some tests to see if comments are working correctly
    void testSimpleExpressionWithComment();
    void testCommentExpression();
    void testMultilineCommandWithComment();

    //tests a syntax error (not closing bracket)
    void testInvalidSyntax();

    void testCompletion();
    void testHelpRequest();
    void testSyntaxHelp();

    //tests variable model
    void testVariablesCreatingFromCode();
    void testVariableCleanupAfterRestart();
    void testVariableCreatingFromCodeWithPlot();

    //tests doing a plot
    void testPlot();

    void testLoginLogout();
    void testRestartWhileRunning();
private:
    QString backendName() override;
};

#endif /* _TESTOCTAVE_H */
