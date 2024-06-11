/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>
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
#ifdef Q_OS_WIN
    m_process->start(QStandardPaths::findExecutable(QLatin1String("cantor_rserver.exe")));
#else
    m_process->start(QStandardPaths::findExecutable(QLatin1String("cantor_rserver")));
#endif

    if (!m_process->waitForStarted())
    {
        changeStatus(Session::Disable);
        emit error(i18n("Failed to start R, please check R installation."));
        emit loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

    m_process->waitForReadyRead();
    qDebug()<<m_process->readAllStandardOutput();

    m_rServer = new org::kde::Cantor::R(QString::fromLatin1("org.kde.Cantor.R-%1").arg(m_process->processId()),  QLatin1String("/"), QDBusConnection::sessionBus(), this);

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
            const int pid = m_process->processId();
            kill(pid, SIGINT);
#else
            ; //TODO: interrupt the process on windows
#endif
        }
        for (auto* expression : expressionQueue())
            expression->setStatus(Cantor::Expression::Interrupted);
        expressionQueue().clear();

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* RSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug()<<"evaluating: "<<cmd;
    auto* expr = new RExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);

    expr->evaluate();

    return expr;
}

Cantor::CompletionObject* RSession::completionFor(const QString& command, int index)
{
    auto* cmp = new RCompletionObject(command, index, this);
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
        auto* expr = expressionQueue().first();
        if (expr->status() == Cantor::Expression::Interrupted)
            return;

        static_cast<RExpression*>(expr)->showFilesAsResult(files);

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

    auto* expr = expressionQueue().first();
    qDebug()<<"running expression: "<<expr->command();

    expr->setStatus(Cantor::Expression::Computing);
    m_rServer->runCommand(expr->internalCommand(), expr->isInternal());
    changeStatus(Cantor::Session::Running);
}

void RSession::sendInputToServer(const QString& input)
{
    QString s = input;
    if(!input.endsWith(QLatin1Char('\n')))
        s += QLatin1Char('\n');
    m_rServer->answerRequest(s);
}

void RSession::inputRequested(QString info)
{
    if (expressionQueue().isEmpty())
        return;

    emit expressionQueue().first()->needsAdditionalInformation(info);
}
