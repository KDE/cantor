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

#include "backendtest.h"

#include "backend.h"
#include "session.h"

#include <KLocale>
#include <QSignalSpy>

void BackendTest::createSession()
{
    Cantor::Backend* b=Cantor::Backend::createBackend( backendName() );
    if(!b || !b->requirementsFullfilled() )
    {
        m_session = 0;
        return;
    }

    m_session=b->createSession();

    QSignalSpy spy(m_session, SIGNAL( ready() ) );
    m_session->login();
    if(spy.isEmpty())
        waitForSignal(m_session, SIGNAL( ready() ) );

    QVERIFY(!spy.isEmpty());
}

Cantor::Expression* BackendTest::evalExp(const QString& exp )
{
   Cantor::Expression* e=m_session->evaluateExpression(exp);

   if(e->status()==Cantor::Expression::Computing)
   {
       waitForSignal( e, SIGNAL( statusChanged( Cantor::Expression::Status ) ) );
   }
   return e;
}

QString BackendTest::cleanOutput(const QString& out)
{
    QString cleaned=out;
    cleaned.replace( QLatin1String("&nbsp;"),QLatin1String(" ") );
    cleaned.remove( QLatin1String("<br/>") );
    cleaned.replace( QChar::ParagraphSeparator, QLatin1Char('\n') );
    cleaned.replace( QRegExp( QLatin1String("\\n\\n") ), QLatin1String("\n") );
    cleaned.replace( QRegExp( QLatin1String("\\n\\s*") ), QLatin1String("\n") );

    return cleaned.trimmed();
}

void BackendTest::initTestCase()
{
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
    {
        m_session->logout();
    }
}

Cantor::Session* BackendTest::session()
{
    return m_session;
}

void BackendTest::waitForSignal(QObject* sender, const char* signal)
{
    QTimer timeout( this );
    timeout.setSingleShot( true );

    QEventLoop loop;
    connect( sender, signal, &loop, SLOT( quit() ) );
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start( 5000 );
    loop.exec();
}



