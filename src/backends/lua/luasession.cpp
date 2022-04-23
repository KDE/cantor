/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
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

LuaSession::LuaSession( Cantor::Backend* backend) : Session(backend),
    m_L(nullptr),
    m_process(nullptr)
{
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

    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readIntroMessage()));
    connect(m_process, SIGNAL(started()), this, SLOT(processStarted()));
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
    while(m_process->bytesAvailable()) {
        m_output.append(QString::fromLocal8Bit(m_process->readLine()));
    }

    const QString& output = m_output.join(QLatin1String("\n"));
    if(!output.isEmpty() && output.trimmed().endsWith(QLatin1String(">"))) {
        qDebug() << " reading the intro message " << m_output ;
        m_output.clear();

        disconnect(m_process, SIGNAL(readyReadStandardOutput()), this , SLOT(readIntroMessage()));
        connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
        connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readError()));
    }
}

void LuaSession::readOutput()
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

    if (expressionQueue().size() > 0)
    {
        LuaExpression* expression = static_cast<LuaExpression*>(expressionQueue().first());

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
        qDebug() << "m_inputCommands" << m_inputCommands;
        qDebug() << m_output;
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

            expression->parseOutput(output);
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
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (expressionQueue().isEmpty() || error.isEmpty())
    {
        return;
    }

    static_cast<LuaExpression*>(expressionQueue().first())->parseError(error);
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
    qDebug() << expressionQueue().size();
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
    }

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* LuaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    changeStatus(Cantor::Session::Running);

    LuaExpression* expression = new LuaExpression(this, internal);

    expression->setFinishingBehavior(behave);
    expression->setCommand(cmd);
    expression->evaluate();

    return expression;
}

void LuaSession::runFirstExpression()
{
    Cantor::Expression* expression = expressionQueue().first();
    connect(expression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionFinished(Cantor::Expression::Status)));
    QString command = expression->internalCommand();

    m_inputCommands = command.split(QLatin1String("\n"));
    m_output.clear();

    command += QLatin1String("\n");
    qDebug() << "final command to be executed " << command;
    qDebug() << "m_inputCommands" << m_inputCommands;
    m_process->write(command.toLocal8Bit());

    expression->setStatus(Cantor::Expression::Computing);
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
    LuaHighlighter* highlighter = new LuaHighlighter(parent);
    return highlighter;
}

lua_State* LuaSession::getState() const
{
    return m_L;
}
