/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/

#include "luasession.h"
#include "luaexpression.h"
#include "luacompletionobject.h"
#include "luahighlighter.h"
#include "luahelper.h"
#include <settings.h>
#include "ui_settings.h"

#ifndef Q_OS_WIN
#include <signal.h>
#endif

#include <QProcess>

LuaSession::LuaSession(Cantor::Backend* backend) : Session(backend) {
}

LuaSession::~LuaSession()
{
    if (m_process)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void LuaSession::login()
{
    emit loginStarted();

    /*
     * setup Qprocess here
     * load the autoscripts
    */

    m_process = new QProcess(this);
    m_process->setProgram(LuaSettings::self()->path().toLocalFile());
    m_process->setArguments(QStringList() << QLatin1String("-i"));

    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &LuaSession::readIntroMessage);
    connect(m_process, &QProcess::started, this, &LuaSession::processStarted);
    m_process->start();
    m_process->waitForStarted();
    m_process->waitForReadyRead();

    // we need this for tab completion
    m_L = luaL_newstate();
    luaL_openlibs(m_L);

    changeStatus(Cantor::Session::Done);
    emit loginDone();
}

void LuaSession::readIntroMessage()
{
    QString output;
    while(m_process->bytesAvailable())
        output += QString::fromLocal8Bit(m_process->readLine()) + QLatin1String("\n");

    if(!output.isEmpty() && output.trimmed().endsWith(QLatin1String(">"))) {
        qDebug() << " reading the intro message " << output ;

        disconnect(m_process, &QProcess::readyReadStandardOutput, this , &LuaSession::readIntroMessage);
        connect(m_process, &QProcess::readyReadStandardError, this, &LuaSession::readError);
        connect(m_process, &QProcess::readyReadStandardOutput, this, &LuaSession::readOutput);
    }
}

void LuaSession::readOutput()
{
    QString output;
    while(m_process->bytesAvailable()) {
        QString line = QString::fromLocal8Bit(m_process->readLine());
        if (line.endsWith(QLatin1String("\n")))
            line.chop(1);

        // join multiple lines with Lua's promt so we can parse the lines as the separate results later
        if (!output.isEmpty())
            output += QLatin1String("> ");
        output += line;
    }

    if (expressionQueue().size() > 0)
    {
        // in the error case Lua sends the prompt with the empty output to stdout _and_ the error message to stderr after this.
        // here we need to remember the expression since the queue is already empty when readError() is called.
        m_lastExpression = static_cast<LuaExpression*>(expressionQueue().first());
        m_lastExpression->parseOutput(output);
    }
}

void LuaSession::readError()
{
    qDebug() << "readError";
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (!m_lastExpression)
        return;

    m_lastExpression->parseError(error);
}

void LuaSession::processStarted()
{
    qDebug() << m_process->program() << " pid   " << m_process->processId() << "  started";
}

void LuaSession::logout()
{
    if (!m_process)
        return;

    if(status() == Cantor::Session::Running)
        interrupt();

    m_process->kill();
    m_process->deleteLater();
    m_process = nullptr;

    Session::logout();
}

void LuaSession::interrupt()
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
    }

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* LuaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    changeStatus(Cantor::Session::Running);

    auto* expression = new LuaExpression(this, internal);

    expression->setFinishingBehavior(behave);
    expression->setCommand(cmd);
    expression->evaluate();

    return expression;
}

void LuaSession::runFirstExpression()
{
    auto* expression = expressionQueue().first();
    connect(expression, &Cantor::Expression::statusChanged, this, &LuaSession::expressionFinished);
    QString command = expression->internalCommand();

    command += QLatin1String("\n");
    qDebug() << "final command to be executed " << command;

    expression->setStatus(Cantor::Expression::Computing);
    m_process->write(command.toLocal8Bit());
}

Cantor::CompletionObject* LuaSession::completionFor(const QString& command, int index)
{
    return new LuaCompletionObject(command, index, this);
}

void LuaSession::expressionFinished(Cantor::Expression::Status status)
{
    switch(status)
    {
        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            finishFirstExpression();
            break;
        default:
            break;
    }
}

QSyntaxHighlighter* LuaSession::syntaxHighlighter(QObject* parent)
{
    return new LuaHighlighter(parent);
}

lua_State* LuaSession::getState() const
{
    return m_L;
}
