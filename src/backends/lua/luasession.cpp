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
    Copyright (C) 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
 */

#include "luasession.h"
#include "luaexpression.h"
#include "luacompletionobject.h"
#include "luahighlighter.h"
#include "luahelper.h"
#include <settings.h>
#include "ui_settings.h"

#include <QProcess>

LuaSession::LuaSession( Cantor::Backend* backend) :
    Session(backend),
    m_process(0),
    m_currentExpression(0)
{
}

LuaSession::~LuaSession()
{
}

void LuaSession::login()
{
    emit loginStarted();

    /*
     * setup Qprocess here
     * load the autoscripts
    */

    m_process = new QProcess(this);
    m_process->setProgram(QLatin1String("/usr/bin/lua"));
    m_process->setArguments(QStringList() << QLatin1String("-i"));

    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readIntroMessage()));
    connect(m_process, SIGNAL(started()), this, SLOT(processStarted()));
    m_process->start();

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

    if(!m_output.isEmpty() && m_output.trimmed().endsWith(QLatin1String(">"))) {
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
            m_output.append(QString::fromLocal8Bit(m_process->readLine()));
    }
    if(m_currentExpression && !m_output.isEmpty() && m_output.trimmed().endsWith(QLatin1String(">"))) {
        // we have our complete output
        // clean the output and parse it and clear m_output;
        m_currentExpression->parseOutput(m_output);
        m_output.clear();

    }

}

void LuaSession::readError()
{
    qDebug() << m_process->readAllStandardError() << endl;
}

void LuaSession::processStarted()
{
    qDebug() << m_process->program() << " pid   " << m_process->processId() << "  started " << endl;
}

void LuaSession::logout()
{
    if(m_process)
        m_process->kill();
}

void LuaSession::interrupt()
{
    // Lua backend is synchronous, there is no way to currently interrupt an expression (in a clean way)
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* LuaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    changeStatus(Cantor::Session::Running);

    m_currentExpression = new LuaExpression(this);
    connect(m_currentExpression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionFinished(Cantor::Expression::Status)));
    m_currentExpression->setFinishingBehavior(behave);
    m_currentExpression->setCommand(cmd);
    m_currentExpression->evaluate();

    return m_currentExpression;
}

void LuaSession::runExpression(LuaExpression *currentExpression)
{
    /*
     * get the current command
     * format it and write to m_process
    */
    QString command = currentExpression->command();

    command += QLatin1String("\n");

    qDebug() << "final command to be executed " << command << endl;

    m_process->write(command.toLocal8Bit());
}

Cantor::CompletionObject* LuaSession::completionFor(const QString& command, int index)
{
    return new LuaCompletionObject(command, index, this);
}

void LuaSession::expressionFinished(Cantor::Expression::Status status)
{
    switch(status) {

        case Cantor::Expression::Computing:
            break;

        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
        case Cantor::Expression::Interrupted:
            changeStatus(Cantor::Session::Done);
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
