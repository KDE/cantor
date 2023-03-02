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
    void cleanupTestCase();

protected:
    void initTestCase();
    Cantor::Expression* evalExp(const QString& exp);
    QString cleanOutput( const QString& out );
    Cantor::Session* session();
    void waitForSignal( QObject* sender, const char* signal);

private:
    void createSession();
    Cantor::Session* m_session{nullptr};
    virtual QString backendName() = 0;
};

#endif // BACKENDTEST_H
