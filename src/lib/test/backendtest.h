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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef BACKENDTEST_H
#define BACKENDTEST_H

#include <QObject>
#include <QtTest>
#include <QtCore>

#include "cantor_export.h"

namespace Cantor
{
    class Session;
    class Expression;
}

class CANTORTEST_EXPORT BackendTest : public QObject
{
    Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

    protected:
        Cantor::Expression* evalExp(const QString& exp);
        /**
         * simple method that removes whitespaces/other irrelevant stuff,
         * so comparing results is easier
         */
        QString cleanOutput( const QString& out );

    private:
        void createSession();
        Cantor::Session* m_session;
        virtual QString backendName() = 0;
};

#endif // BACKENDTEST_H
