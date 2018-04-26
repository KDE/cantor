/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2018 Nikita Sirgienko <warquark@gmail.com>
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
    //tests simple variable difinition
    void testVariableDifinition();
    //tests a syntax error (not closing bracket)
    void testInvalidSyntax();
    //tests if-else condition
    void testIfElseCondition();
    //tests 'for' loop
    void testForLoop();

private:
    QString backendName() Q_DECL_OVERRIDE;
};

#endif /* _TESTLUA_H */

