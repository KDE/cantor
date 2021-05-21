/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _TESTR_H
#define _TESTR_H

#include "backendtest.h"

class TestR : public BackendTest
{
  Q_OBJECT
private Q_SLOTS:
    void testSimpleCommand();
    void testMultilineCommand();
    void testCommandQueue();

    void testVariableDefinition();
    void testMatrixDefinition();

    void testCodeWithComments();
    void testCommentExpression();
    void testMultilineCommandWithComment();

    void testInvalidSyntax();

    void testCompletion();
    void testHelpRequest();
    void testSyntaxHelp();
    void testInformationRequest();

    void testSimplePlot();
    void testComplexPlot();

    //tests variable model
    void testVariablesCreatingFromCode();
    void testVariableCleanupAfterRestart();

    void testLoginLogout();
    void testRestartWhileRunning();
private:
    QString backendName() override;
};

#endif /* _TESTR_H */
