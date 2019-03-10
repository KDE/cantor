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

#include "testr.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "completionobject.h"
#include "defaultvariablemodel.h"

#include <QDebug>


QString TestR::backendName()
{
    return QLatin1String("R");
}

void TestR::testVariablesCreatingFromCode()
{
    QAbstractItemModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    Cantor::Expression* e=evalExp(QLatin1String("a1 = 15; b1 = 'S'; d1 = c(1,2,3)"));
    QVERIFY(e!=nullptr);

    while(session()->status() != Cantor::Session::Done)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(3, model->rowCount());

    QCOMPARE(model->index(0,0).data().toString(), QLatin1String("a1"));
    QCOMPARE(model->index(0,1).data().toString(), QLatin1String("15"));

    QCOMPARE(model->index(1,0).data().toString(), QLatin1String("b1"));
    QCOMPARE(model->index(1,1).data().toString(), QLatin1String("S"));

    QCOMPARE(model->index(2,0).data().toString(), QLatin1String("d1"));
    QCOMPARE(model->index(2,1).data().toString(), QLatin1String("1, 2, 3"));

    evalExp(QLatin1String("rm(a1,b1,d1)"));
}

void TestR::testVariableCleanupAfterRestart()
{
    Cantor::DefaultVariableModel* model = session()->variableModel();
    QVERIFY(model != nullptr);

    while(session()->status() != Cantor::Session::Done)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(static_cast<QAbstractItemModel*>(model)->rowCount(), 0);

    Cantor::Expression* e=evalExp(QLatin1String("h1 = 15; h2 = 'S';"));
    QVERIFY(e!=nullptr);

    while(session()->status() != Cantor::Session::Done)
        waitForSignal(session(), SIGNAL(statusChanged(Cantor::Session::Status)));

    QCOMPARE(static_cast<QAbstractItemModel*>(model)->rowCount(), 2);

    session()->logout();
    session()->login();

    QCOMPARE(static_cast<QAbstractItemModel*>(model)->rowCount(), 0);
}

QTEST_MAIN( TestR )

