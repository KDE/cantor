/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "backendtest.h"

#include "backend.h"
#include "session.h"
#include "../../config-cantor.h"

#include <KLocalizedString>
#include <QSignalSpy>
#include <QDebug>

void BackendTest::createSession()
{
    // Add our custom plugins path, where we install our plugins, if it isn't default path
    const QString& path = QString::fromLocal8Bit(PATH_TO_CANTOR_PLUGINS);
    qDebug() << "Adding additional application library path" << path;
    if (!QCoreApplication::libraryPaths().contains(path))
        QCoreApplication::addLibraryPath(path);

    auto* b = Cantor::Backend::getBackend( backendName() );
    if(!b || !b->requirementsFullfilled() )
    {
        m_session = nullptr;
        return;
    }

    m_session = b->createSession();

    QSignalSpy spy(m_session, SIGNAL(loginDone()) );
    QSignalSpy spyError(m_session, SIGNAL(error(const QString&)) );
    m_session->login();
    if(spy.isEmpty())
        waitForSignal(m_session, SIGNAL(loginDone()) );

    QVERIFY(!spy.isEmpty());
    QVERIFY(spyError.isEmpty());
}

Cantor::Expression* BackendTest::evalExp(const QString& exp )
{
   auto* e = m_session->evaluateExpression(exp);

   if(e->status() == Cantor::Expression::Queued)
       waitForSignal( e, SIGNAL(statusChanged(Cantor::Expression::Status)) );

   if (e->status() == Cantor::Expression::Computing)
       waitForSignal( e, SIGNAL(statusChanged(Cantor::Expression::Status)) );

   return e;
}

/**
* simple method that removes whitespaces/other irrelevant stuff,
* so comparing results is easier
*/
QString BackendTest::cleanOutput(const QString& out)
{
    QString cleaned = out;
    cleaned.replace( QLatin1String("&nbsp;"),QLatin1String(" ") );
    cleaned.remove( QLatin1String("<br/>") );
    cleaned.replace( QChar::ParagraphSeparator, QLatin1Char('\n') );
    cleaned.replace( QRegularExpression( QStringLiteral("\\n{2}") ), QStringLiteral("\n") );
    cleaned.replace( QRegularExpression( QStringLiteral("\\n\\s*") ), QStringLiteral("\n") );

    return cleaned.trimmed();
}

void BackendTest::initTestCase()
{
    QCoreApplication::setApplicationName(QLatin1String("cantor"));
    createSession();
    if (!m_session)
    {
        QString reason = i18n("This test requires a functioning %1 backend", backendName() );
        QSKIP( reason.toStdString().c_str(), SkipAll );
    }
}

void BackendTest::cleanupTestCase()
{
    if (m_session)
        m_session->logout();
}

Cantor::Session* BackendTest::session()
{
    return m_session;
}

/**
* simple method that blocks and waits for a signal to be emitted
*/
void BackendTest::waitForSignal(QObject* sender, const char* signal)
{
    QTimer timeout( this );
    timeout.setSingleShot( true );

    QEventLoop loop;
    connect( sender, signal, &loop, SLOT(quit()) );
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start( 25000 );
    loop.exec();
}
