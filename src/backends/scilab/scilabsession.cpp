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
    Copyright (C) 2011 Filipe Saraiva <filip.saraiva@gmail.com>
 */

#include "scilabsession.h"
#include "scilabexpression.h"

#include <kdebug.h>
#include <KProcess>
#include <settings.h>

ScilabSession::ScilabSession( Cantor::Backend* backend) : Session(backend)
{
    m_process = 0;
    kDebug();
}

ScilabSession::~ScilabSession()
{
    kDebug();
}

void ScilabSession::login()
{
    kDebug()<<"login";

    m_process = new KProcess(this);
    m_process->setProgram(ScilabSettings::self()->path().toLocalFile());

    kDebug() << m_process->program();

    m_process->setOutputChannelMode(KProcess::ForwardedChannels);
    QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(readOutput()));
    QObject::connect(m_process, SIGNAL(readyReadStandardError()), SLOT (readError()));

    m_process->start();
    emit ready();
}

void ScilabSession::logout()
{
    kDebug()<<"logout";

    m_process->write("exit\n");
    m_process->kill();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

void ScilabSession::interrupt()
{
    kDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* ScilabSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug() << "evaluating: " << cmd;
    ScilabExpression* expr = new ScilabExpression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void ScilabSession::runExpression(ScilabExpression* expr)
{
    QString command = expr->command();
    m_currentExpression = expr;

    connect(m_currentExpression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionStatusChanged(Cantor::Expression::Status)));

    kDebug() << "Writing command to process" << command;

    command += '\n';
    m_process->write(command.toUtf8());

    readOutput();
}

void ScilabSession::expressionFinished()
{
    kDebug()<<"finished";
    ScilabExpression* expression = qobject_cast<ScilabExpression*>(sender());

    m_runningExpressions.removeAll(expression);
    kDebug() << "size: " << m_runningExpressions.size();
}

void ScilabSession::readError()
{
    kDebug() << "readError";

    QString error = m_process->readAllStandardError();

    kDebug() << "error: " << error;
}

void ScilabSession::readOutput()
{
    kDebug() << "readOutput";

    QString output = m_process->readAllStandardOutput();
    output += " output\n";

    kDebug() << "output: " << output;

    m_currentExpression->parseOutput(output);
}

void ScilabSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    kDebug() << "currentExpressionStatusChanged: " << status;

    switch (status)
    {
        case Cantor::Expression::Computing:
            break;

        case Cantor::Expression::Interrupted:
            break;

        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            changeStatus(Done);

            break;
    }
}

#include "scilabsession.moc"
