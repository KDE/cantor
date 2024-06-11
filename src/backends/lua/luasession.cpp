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

const QString LuaSession::LUA_PROMPT = QLatin1String("> ");
const QString LuaSession::LUA_SUBPROMPT = QLatin1String(">> ");

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

    m_process = new QProcess(this);

    const auto& path = LuaSettings::self()->path().toLocalFile();
    QFileInfo fi(path);
    if (fi.baseName() != QLatin1String("luajit"))
        m_luaJIT = false;

    m_process->setProgram(path);
    m_process->setArguments(QStringList() << QLatin1String("-i"));
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &LuaSession::readIntroMessage);
    connect(m_process, &QProcess::started, this, &LuaSession::processStarted);
    m_process->start();

    if (!m_process->waitForStarted())
    {
        changeStatus(Session::Disable);
        emit error(i18n("Failed to start Lua, please check Lua installation."));
        emit loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

    m_process->waitForReadyRead();

    // TODO: load the auto-scripts

    // we need this for tab completion
    m_L = luaL_newstate();
    luaL_openlibs(m_L);

    changeStatus(Cantor::Session::Done);
    emit loginDone();
}

bool LuaSession::isLuaJIT() const
{
    return m_luaJIT;
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
    if (m_luaJIT)
        readOutputLuaJIT();
    else
        readOutputLua();
}

void LuaSession::readOutputLuaJIT()
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

    if (m_lastExpression)
        m_lastExpression->parseOutput(output);
}

void LuaSession::readOutputLua()
{
    /*
     * parse the output
     * clear all the garbage
     * set it as output
    */
    // keep reading till the output ends with '>'.
    // '>' marks the end of output for a particular command;
    while(m_process->bytesAvailable()) {
        QString line = QString::fromLocal8Bit(m_process->readLine());
        if (line.endsWith(QLatin1String("\n")))
            line.chop(1);
        m_output.append(line);
    }

    qDebug()<<"parsing the initial output of Lua " << m_output;

    if (expressionQueue().size() > 0)
    {
        // How parsing works:
        // For each line of input command, which for example we name as X
        // lua prints output in this form (common form, 90% of output)
        // X + "\n" + command_output + "\n" + "> " or ">> " + "\n"
        // or (merged form, rare, only 10% of output
        // X + "\n" + command_output + "\n" + ("> " or ">> " in beginning of next X)
        // Sometimes empty lines also apears in command output

        // In this realisation we iterate over input lines and output line and copy only output lines
        int input_idx = 0;
        int previous_output_idx = 0;
        int output_idx = 0;
        QString output;
        // qDebug() << "m_inputCommands" << m_inputCommands;
        while (output_idx < m_output.size() && input_idx < m_inputCommands.size())
        {
            const QString& line = m_output[output_idx];
            bool previousLineIsPrompt = (output_idx >= 1 ? isPromptString(m_output[output_idx-1]) : true);
            bool commonInputMatch = (line == m_inputCommands[input_idx] && previousLineIsPrompt);
            bool mergedInputMatch = (line == LUA_PROMPT + m_inputCommands[input_idx] || line == LUA_SUBPROMPT + m_inputCommands[input_idx]);

            if (commonInputMatch || mergedInputMatch)
            {
                if (previous_output_idx + 1 < output_idx)
                {
                    for (int i = previous_output_idx+1; i < output_idx; i++)
                    {
                        const QString& copiedLine = m_output[i];
                        bool isLastLine = i == output_idx-1;
                        if (!(isLastLine && previousLineIsPrompt))
                            output += copiedLine + QLatin1String("\n");
                    }
                }
                previous_output_idx = output_idx;
                input_idx++;
            }
            output_idx++;
        }

        if (input_idx == m_inputCommands.size() && m_output[m_output.size()-1] == LUA_PROMPT)
        {
            //We parse all output, so copy tail of the output and pass it to lua expression
            for (int i = previous_output_idx+1; i < m_output.size()-1; i++)
            {
                const QString& copiedLine = m_output[i];
                bool isLastLine = i == m_output.size()-1;
                if (isLastLine)
                    output += copiedLine;
                else
                    output += copiedLine + QLatin1String("\n");
            }

            if (m_lastExpression)
                m_lastExpression->parseOutput(output);
            m_output.clear();
        }
    }
}

bool LuaSession::isPromptString(const QString& s)
{
    return s == LUA_PROMPT || s == LUA_SUBPROMPT;
}

void LuaSession::readError()
{
    qDebug() << "readError";
    if (!m_lastExpression)
        return;

    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
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
    // in the error case LuaJIT sends the prompt with the empty output to stdout _and_ the error message to stderr after this.
    // here we need to remember the expression since the queue is already empty when readError() is called.
    m_lastExpression = static_cast<LuaExpression*>(expressionQueue().first());

    connect(m_lastExpression, &Cantor::Expression::statusChanged, this, &LuaSession::expressionFinished);
    QString command = m_lastExpression->internalCommand();

    m_inputCommands = command.split(QLatin1String("\n"));
    m_output.clear();

    command += QLatin1String("\n");
    qDebug() << "final command to be executed " << command;

    m_lastExpression->setStatus(Cantor::Expression::Computing);
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
