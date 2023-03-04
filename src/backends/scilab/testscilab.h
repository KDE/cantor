/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
*/

#ifndef TESTSCILAB_H
#define TESTSCILAB_H

#include "backendtest.h"

// This class tests some of the basic functions of the Scilab backend

class TestScilab : public BackendTest
{
  Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testSimpleCommand();
    void testVariableDefinition();
    void testInvalidSyntax();
    void testLoginLogout();
    void testPlot();

private:
    QString backendName() override;
};

#endif // TESTSCILAB_H
