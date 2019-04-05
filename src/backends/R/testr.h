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
    Copyright (C) 2019 Sirgienko Nikita <warquark@gmail.com>
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
    //void testHelpRequest();
    //void testSyntaxHelp();

    //void testPlot();

    //tests variable model
    void testVariablesCreatingFromCode();
    void testVariableCleanupAfterRestart();

private:
    QString backendName() override;
};

#endif /* _TESTR_H */
