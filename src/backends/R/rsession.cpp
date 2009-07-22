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

#include "rsession.h"
#include "rexpression.h"

#include <QTimer>
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include <signal.h>

RSession::RSession( MathematiK::Backend* backend) : Session(backend)
{
    kDebug();
    m_rProcess=0;
}

RSession::~RSession()
{
    kDebug();
    m_rProcess->terminate();
}

void RSession::login()
{
    kDebug()<<"login";
    if(m_rProcess)
        m_rProcess->deleteLater();
    m_rProcess=new KProcess(this);
    m_rProcess->setOutputChannelMode(KProcess::ForwardedChannels);

    (*m_rProcess)<<KStandardDirs::findExe( "mathematik_rserver" );

    m_rProcess->start();

    m_rServer=new org::MathematiK::R(QString("org.kde.mathematik_rserver-%1").arg(m_rProcess->pid()),  "/R", QDBusConnection::sessionBus(), this);

    connect(m_rServer, SIGNAL(statusChanged(int)), this, SLOT(serverChangedStatus(int)));

    changeStatus(MathematiK::Session::Done);

    connect(m_rServer, SIGNAL(ready()), this, SIGNAL(ready()));
}

void RSession::logout()
{
    kDebug()<<"logout";
    m_rProcess->terminate();
}

void RSession::interrupt()
{
    kDebug()<<"interrupt";
    kill(m_rProcess->pid(), 2);
    changeStatus(MathematiK::Session::Done);
}

MathematiK::Expression* RSession::evaluateExpression(const QString& cmd)
{
    kDebug()<<"evaluating: "<<cmd;
    RExpression* expr=new RExpression(this);

    expr->setCommand(cmd);

    expr->evaluate();

    changeStatus(MathematiK::Session::Running);

    return expr;
}

void RSession::queueExpression(RExpression* expr)
{
    m_expressionQueue.append(expr);

    if(status()==MathematiK::Session::Done)
        QTimer::singleShot(0, this, SLOT(runNextExpression()));
}


void RSession::serverChangedStatus(int status)
{
    kDebug()<<"changed status to "<<status;
    if(status==0)
        changeStatus(MathematiK::Session::Done);
    else
        changeStatus(MathematiK::Session::Running);
}

void RSession::runNextExpression()
{
    disconnect(m_rServer,  SIGNAL(expressionFinished(int, const QString&)),  0,  0);
    disconnect(m_rServer, SIGNAL(inputRequested(const QString&)), 0, 0);
    disconnect(m_rServer, SIGNAL(showFilesNeeded(const QStringList&)), 0, 0);
    RExpression* expr=m_expressionQueue.takeFirst();
    kDebug()<<"running expression: "<<expr->command();

    connect(m_rServer, SIGNAL(expressionFinished(int,  const QString &)), expr, SLOT(finished(int, const QString&)));
    connect(m_rServer, SIGNAL(inputRequested(const QString&)), expr, SIGNAL(needsAdditionalInformation(const QString&)));
    connect(m_rServer, SIGNAL(showFilesNeeded(const QStringList&)), expr, SLOT(showFilesAsResult(const QStringList&)));

    m_rServer->runCommand(expr->command());

    if(!m_expressionQueue.isEmpty())
        QTimer::singleShot(0, this, SLOT(runNextExpression()));
}

void RSession::sendInputToServer(const QString& input)
{
    QString s=input;
    if(!input.endsWith('\n'))
        s+='\n';
    m_rServer->answerRequest(s);
}

#include "rsession.moc"
