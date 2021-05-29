/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _TESTSAGE_H
#define _TESTSAGE_H

#include "backendtest.h"

/** This class test some of the basic functions of the sage backend
    The different tests represent some general expressions, as well
    as expressions, that are known to have caused problems in earlier
    versions
**/
class TestSage : public BackendTest
{
  Q_OBJECT

private Q_SLOTS:
    //tests evaluating a simple command
    void testSimpleCommand();

    //tests if the backend gets confused if more than
    //one command is in the queue
    void testCommandQueue();
    //tests a command, containing more than 1 line
    void testMultilineCommand();

    //tests defining and calling a function
    void testDefineFunction();

    //tests doing a plot
    void testPlot();

    //tests a syntax error (not closing bracket)
    void testInvalidSyntax();

    //tests a two-line command, where one doesn't deliver output
    //(source of problem in earlier versions)
    void testNoOutput();

    void testLoginLogout();
    void testRestartWhileRunning();
private:
    QString backendName() override;

};

#endif /* _TESTSAGE_H */
