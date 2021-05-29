/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#pragma once

#include <backendtest.h>

class TestJulia: public BackendTest
{
    Q_OBJECT
private Q_SLOTS:
    /// Test simple one-line command. Check that last result is printed
    void testOneLine();
    /// Test one-line command returning `nothing`. No result is printed, except what `print` does
    void testOneLineWithPrint();
    /// Test command, that emits exception
    void testException();
    /// Test command consisting of multiple lines, including comments.
    void testMultilineCode();
    /// Test command with malformed syntax
    void testSyntaxError();
    /// Test that results gathered before exception occurred are shown
    void testPartialResultOnException();
    /// Test command queue with some simple expressions
    void testExpressionQueue();

    /// Tests that inline plot is shown
    void testInlinePlot();
    /// Tests that when exception occurred and plotting is done, partial text results shown to user
    void testInlinePlotWithExceptionAndPartialResult();

    /// Test registering new variables, when added by command
    void testAddVariablesFromCode();
    /// Test registering new variables, when added from variable manager
    void testAddVariablesFromManager();
    /// Test that removing variable unregisters it
    void testRemoveVariables();

    /// Test that auto completion provides expected results
    void testAutoCompletion();
    void testComplexAutocompletion();

    void testHelpRequest();

    void testLoginLogout();
    void testRestartWhileRunning();
private:
    QString backendName() override;
};
