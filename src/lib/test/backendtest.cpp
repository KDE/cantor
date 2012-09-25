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

void BackendTest::createSession()
{
    Cantor::Backend* b=Cantor::Backend::createBackend( backendName() );
    if(!b || !b->requirementsFullfilled() )
    {
        m_session = 0;
        return;
    }

    m_session=b->createSession();
    m_session->login();

    QEventLoop loop;
    connect( m_session, SIGNAL( ready() ), &loop, SLOT( quit() ) );
    loop.exec();
}

Cantor::Expression* BackendTest::evalExp(const QString& exp )
{
   Cantor::Expression* e=m_session->evaluateExpression(exp);

   if(e->status()==Cantor::Expression::Computing)
   {
       //Create a timeout, that kills the eventloop, if the expression doesn't finish
       QTimer timeout( this );
       timeout.setSingleShot( true );
       timeout.start( 5000 );
       QEventLoop loop;
       connect( &timeout, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
       connect( e, SIGNAL( statusChanged( Cantor::Expression::Status ) ), &loop, SLOT( quit() ) );

       loop.exec();
   }
   return e;
}

QString BackendTest::cleanOutput(const QString& out)
{
    QString cleaned=out;
    cleaned.replace( "&nbsp;"," " );
    cleaned.remove( "<br/>" );
    cleaned.replace( QChar::ParagraphSeparator, '\n' );
    cleaned.replace( QRegExp( "\\n\\n" ), "\n" );
    cleaned.replace( QRegExp( "\\n\\s*" ), "\n" );

    return cleaned.trimmed();
}

void BackendTest::initTestCase()
{
    createSession();
    if (!m_session)
    {
        QString reason = i18n("This test requires a functioning %1 backend", backendName() );
        QSKIP( reason.toLatin1(), SkipAll );
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


#include "backendtest.moc"
