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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#include "testpython3.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"

QString TestPython3::backendName()
{
    return QLatin1String("python3");
}

void TestPython3::testImportNumpy()
{
    Cantor::Expression* e = evalExp(QLatin1String("import numpy"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result() != nullptr);
}

void TestPython3::testCodeWithComments()
{
    {
    Cantor::Expression* e = evalExp(QLatin1String("#comment\n1+2"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }

    {
    Cantor::Expression* e = evalExp(QLatin1String("     #comment\n1+2"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }
}

void TestPython3::testPython3Code()
{
    {
    Cantor::Expression* e = evalExp(QLatin1String("print 1 + 2"));

    QVERIFY(e != nullptr);
    QVERIFY(!e->errorMessage().isEmpty());
    }

    {
    Cantor::Expression* e = evalExp(QLatin1String("print(1 + 2)"));

    QVERIFY(e != nullptr);
    QVERIFY(e->result()->data().toString() == QLatin1String("3"));
    }
}

QTEST_MAIN(TestPython3)

