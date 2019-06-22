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
    Copyright (C) 2018 Alexander Semke <alexander.semke@web.de>
 */

#include "rsession.h"
#include "rexpression.h"
#include "rcompletionobject.h"
#include "rhighlighter.h"
#include "rvariablemodel.h"
#include <defaultvariablemodel.h>

#include <QTimer>
#include <QDebug>
#include <KProcess>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

RSession::RSession(Cantor::Backend* backend) : Session(backend),
m_process(nullptr),
m_rServer(nullptr)
{
    setVariableModel(new RVariableModel(this));
}

RSession::~RSession()
{
    if (m_process)
        m_process->terminate();
}

void RSession::login()
{
    qDebug()<<"login";
    if (m_process)
        return;
    emit loginStarted();

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    m_process->start(QStandardPaths::findExecutable(QLatin1String("cantor_rserver")));

    m_process->waitForStarted();
    m_process->waitForReadyRead();
    qDebug()<<m_process->readAllStandardOutput();

    m_rServer = new org::kde::Cantor::R(QString::fromLatin1("org.kde.Cantor.R-%1").arg(m_process->pid()),  QLatin1String("/"), QDBusConnection::sessionBus(), this);

    connect(m_rServer, &org::kde::Cantor::R::statusChanged, this, &RSession::serverChangedStatus);
    connect(m_rServer,  &org::kde::Cantor::R::expressionFinished, this, &RSession::expressionFinished);
    connect(m_rServer, &org::kde::Cantor::R::inputRequested, this, &RSession::inputRequested);

    changeStatus(Session::Done);
    emit loginDone();
    qDebug()<<"login done";
}

void RSession::logout()
{
    qDebug()<<"logout";
    if (!m_process)
        return;

    if(status() == Cantor::Session::Running)
        interrupt();

    m_process->kill();
    m_process->deleteLater();
    m_process = nullptr;

    Session::logout();
}

void RSession::interrupt()
{
    if(!expressionQueue().isEmpty())
    {
        qDebug()<<"interrupting " << expressionQueue().first()->command();
        if(m_process && m_process->state() != QProcess::NotRunning)
        {
#ifndef Q_OS_WIN
            const int pid=m_process->pid();
            kill(pid, SIGINT);
#else
            ; //TODO: interrupt the process on windows
#endif
        }
        foreach (Cantor::Expression* expression, expressionQueue())
            expression->setStatus(Cantor::Expression::Interrupted);
        expressionQueue().clear();

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* RSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug()<<"evaluating: "<<cmd;
    RExpression* expr=new RExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);

    expr->evaluate();

    return expr;
}

Cantor::CompletionObject* RSession::completionFor(const QString& command, int index)
{
    RCompletionObject *cmp=new RCompletionObject(command, index, this);
    return cmp;
}

QSyntaxHighlighter* RSession::syntaxHighlighter(QObject* parent)
{
    return new RHighlighter(parent, this);
}

void RSession::serverChangedStatus(int status)
{
    qDebug()<<"changed status to "<<status;
    if(status==0)
    {
        if(expressionQueue().isEmpty())
            changeStatus(Cantor::Session::Done);
    }
    else
        changeStatus(Cantor::Session::Running);
}

void RSession::expressionFinished(int returnCode, const QString& text, const QStringList& files)
{
    if (!expressionQueue().isEmpty())
    {
        RExpression* expr = static_cast<RExpression*>(expressionQueue().first());
        if (expr->status() == Cantor::Expression::Interrupted)
            return;

        if (!files.isEmpty())
            expr->showFilesAsResult(files);

        if(returnCode==RExpression::SuccessCode)
            expr->parseOutput(text);
        else if (returnCode==RExpression::ErrorCode)
            expr->parseError(text);

        qDebug()<<"done running "<<expr<<" "<<expr->command();
        finishFirstExpression();
    }
}

void RSession::runFirstExpression()
{
    if (expressionQueue().isEmpty())
        return;

    RExpression* expr = static_cast<RExpression*>(expressionQueue().first());
    qDebug()<<"running expression: "<<expr->command();

    expr->setStatus(Cantor::Expression::Computing);
    m_rServer->runCommand(expr->internalCommand(), expr->isInternal());
    changeStatus(Cantor::Session::Running);
}

void RSession::sendInputToServer(const QString& input)
{
    QString s=input;
    if(!input.endsWith(QLatin1Char('\n')))
        s+=QLatin1Char('\n');
    m_rServer->answerRequest(s);
}

void RSession::inputRequested(QString info)
{
    if (expressionQueue().isEmpty())
        return;

    emit expressionQueue().first()->needsAdditionalInformation(info);
}
