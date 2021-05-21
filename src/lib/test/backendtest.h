/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef BACKENDTEST_H
#define BACKENDTEST_H

#include <QObject>
#include <QtTest>
#include <QString>

#include <cantortest_export.h>

namespace Cantor
{
    class Session;
    class Expression;
}

class CANTORTEST_EXPORT BackendTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();

    protected:
        Cantor::Expression* evalExp(const QString& exp);
        /**
         * simple method that removes whitespaces/other irrelevant stuff,
         * so comparing results is easier
         */
        QString cleanOutput( const QString& out );

	Cantor::Session* session();

	/**
	 * simple method that blocks and waits for a signal to be emitted
	 */
	void waitForSignal( QObject* sender, const char* signal);
    private:
        void createSession();
        Cantor::Session* m_session;
        virtual QString backendName() = 0;
};

#endif // BACKENDTEST_H
