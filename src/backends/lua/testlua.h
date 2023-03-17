/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
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
    void testSimpleCommand();
    void testMultilineCommand();
    void testVariableDefinition();
    void testInvalidSyntax();
    void testIfElseCondition();
    void testForLoop();
    void testFunction();

private:
    QString backendName() override;
};

#endif /* _TESTLUA_H */

