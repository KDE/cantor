/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#ifndef _TESTPYTHON3_H
#define _TESTPYTHON3_H

#include <backendtest.h>

class TestPython3 : public BackendTest
{
  Q_OBJECT
  private Q_SLOTS:
    void testSimpleCommand();
    void testMultilineCommand();
    void testCodeWithComments();
    void testCommandQueue();

    void testSimplePlot();
    void testPlotWithIPythonMagic();

    void testImportStatement();
    void testPython3Code();
    void testInvalidSyntax();

    void testSimpleExpressionWithComment();
    void testCommentExpression();
    void testMultilineCommandWithComment();

    void testVariablesCreatingFromCode();
    void testVariableChangeSizeType();
    void testVariableCleanupAfterRestart();
    void testDictVariable();

    void testCompletion();
    void testInterrupt();

    void testWarning();
  private:
    QString backendName() override;
};

#endif /* _TESTPYTHON3_H */
