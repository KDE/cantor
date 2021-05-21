/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef _TESTLUA_H
#define _TESTLUA_H

#include "backendtest.h"

/** This class test some of the basic functions of the Lua backend
    The different tests represent some general expressions for preventing possible future regression
**/
class TestLua : public BackendTest
{
  Q_OBJECT

private Q_SLOTS:
    //tests evaluating a simple command
    void testSimpleCommand();
    //tests a command, containing more than 1 line
    void testMultilineCommand();
    //tests simple variable definition
    void testVariableDefinition();
    //tests a syntax error (not closing bracket)
    void testInvalidSyntax();
    //tests if-else condition
    void testIfElseCondition();
    //tests 'for' loop
    void testForLoop();

private:
    QString backendName() override;
};

#endif /* _TESTLUA_H */

