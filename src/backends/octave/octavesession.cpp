/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#include "octavesession.h"
#include "octaveexpression.h"
#include "octavecompletionobject.h"
#include "octavesyntaxhelpobject.h"
#include "octavehighlighter.h"
#include "result.h"
#include "textresult.h"

#include "settings.h"
#include "octavehighlighter.h"

#include <KProcess>
#include <KDirWatch>
#include <KLocalizedString>

#include <QTimer>
#include <QFile>
#include <QDir>
#include <QStringRef>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

#include "octavevariablemodel.h"

const QRegExp OctaveSession::PROMPT_UNCHANGEABLE_COMMAND = QRegExp(QLatin1String("(,|;)+"));

OctaveSession::OctaveSession ( Cantor::Backend* backend ) : Session ( backend),
m_process(nullptr),
m_prompt(QLatin1String("CANTOR_OCTAVE_BACKEND_PROMPT:([0-9]+)> ")),
m_subprompt(QLatin1String("CANTOR_OCTAVE_BACKEND_SUBPROMPT:([0-9]+)> ")),
m_previousPromptNumber(1),
m_syntaxError(false)
{
    setVariableModel(new OctaveVariableModel(this));
}

OctaveSession::~OctaveSession()
{
    if (m_process)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void OctaveSession::login()
{
    qDebug() << "login";
    if (m_process)
        return;

    emit loginStarted();

    m_process = new KProcess ( this );
    QStringList args;
    args << QLatin1String("--silent");
    args << QLatin1String("--interactive");
    args << QLatin1String("--persist");

    // Setting prompt and subprompt
    args << QLatin1String("--eval");
    args << QLatin1String("PS1('CANTOR_OCTAVE_BACKEND_PROMPT:\\#> ');");
    args << QLatin1String("--eval");
    args << QLatin1String("PS2('CANTOR_OCTAVE_BACKEND_SUBPROMPT:\\#> ');");

    // Add the cantor script directory to octave script search path
    const QStringList& scriptDirs = locateAllCantorFiles(QLatin1String("octavebackend"), QStandardPaths::LocateDirectory);
    if (scriptDirs.isEmpty())
        qCritical() << "Octave script directory not found, needed for integrated plots";
    else
    {
        for (const QString& dir : scriptDirs)
            args << QLatin1String("--eval") << QString::fromLatin1("addpath \"%1\";").arg(dir);
    }

    // Do not show extra text in help commands
    args << QLatin1String("--eval");
    args << QLatin1String("suppress_verbose_help_message(1);");

    if (OctaveSettings::integratePlots())
    {
        // Do not show the popup when plotting, rather only print to a file
        args << QLatin1String("--eval");
        args << QLatin1String("set (0, \"defaultfigurevisible\",\"off\");");
        args << QLatin1String("--eval");
        args << QLatin1String("graphics_toolkit gnuplot;");
    }
    else
    {
        args << QLatin1String("--eval");
        args << QLatin1String("set (0, \"defaultfigurevisible\",\"on\");");
    }

    m_process->setProgram ( OctaveSettings::path().toLocalFile(), args );
    qDebug() << "starting " << m_process->program();
    m_process->setOutputChannelMode ( KProcess::SeparateChannels );
    m_process->start();
    m_process->waitForStarted();

    connect ( m_process, SIGNAL (readyReadStandardOutput()), SLOT (readOutput()) );
    connect ( m_process, SIGNAL (readyReadStandardError()), SLOT (readError()) );
    connect ( m_process, SIGNAL (error(QProcess::ProcessError)), SLOT (processError()) );

    if(!OctaveSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = OctaveSettings::self()->autorunScripts().join(QLatin1String("\n"));

        evaluateExpression(autorunScripts, OctaveExpression::DeleteOnFinish, true);
        updateVariables();
    }

    changeStatus(Cantor::Session::Done);
    emit loginDone();
    qDebug()<<"login done";
}

void OctaveSession::logout()
{
    qDebug()<<"logout";

    if(!m_process)
        return;

    disconnect(m_process, nullptr, this, nullptr);

    if(status() == Cantor::Session::Running)
        interrupt();

    m_process->write("exit\n");
    qDebug()<<"send exit command to octave";

    if(!m_process->waitForFinished(1000))
    {
        m_process->kill();
        qDebug()<<"octave still running, process kill enforced";
    }
    m_process->deleteLater();
    m_process = nullptr;

    expressionQueue().clear();

    m_output.clear();
    m_previousPromptNumber = 1;

    variableModel()->clearVariables();

    changeStatus(Status::Disable);

    qDebug()<<"logout done";
}

void OctaveSession::interrupt()
{
    qDebug() << expressionQueue().size();
    if(!expressionQueue().isEmpty())
    {
        qDebug()<<"interrupting " << expressionQueue().first()->command();
        if(m_process->state() != QProcess::NotRunning)
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

        // Cleanup inner state and call octave prompt printing
        // If we move this code for interruption to Session, we need add function for
        // cleaning before setting Done status
        m_output.clear();
        m_process->write("\n");

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

void OctaveSession::processError()
{
    qDebug() << "processError";
    emit error(m_process->errorString());
}

Cantor::Expression* OctaveSession::evaluateExpression ( const QString& command, Cantor::Expression::FinishingBehavior finishingBehavior, bool internal )
{
    qDebug() << "evaluating: " << command;
    OctaveExpression* expression = new OctaveExpression ( this, internal);
    expression->setCommand ( command );
    expression->setFinishingBehavior ( finishingBehavior );
    expression->evaluate();

    return expression;
}

void OctaveSession::runFirstExpression()
{
    OctaveExpression* expression = static_cast<OctaveExpression*>(expressionQueue().first());
    connect(expression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionStatusChanged(Cantor::Expression::Status)));
    QString command = expression->internalCommand();
    expression->setStatus(Cantor::Expression::Computing);
    if (isDoNothingCommand(command))
        expression->setStatus(Cantor::Expression::Done);
    else
    {
        m_process->write ( command.toLocal8Bit() );
    }
}

void OctaveSession::readError()
{
    qDebug() << "readError";
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (!expressionQueue().isEmpty() && !error.isEmpty())
    {
        OctaveExpression* const exp = static_cast<OctaveExpression*>(expressionQueue().first());
        if (m_syntaxError)
        {
            m_syntaxError = false;
            exp->parseError(i18n("Syntax Error"));
        }
        else
            exp->parseError(error);

        m_output.clear();
    }
}

void OctaveSession::readOutput()
{
    qDebug() << "readOutput";
    while (m_process->bytesAvailable() > 0)
    {
        QString line = QString::fromLocal8Bit(m_process->readLine());
        qDebug()<<"start parsing " << "  " << line;
        if (line.contains(m_prompt))
        {
            const int promptNumber = m_prompt.cap(1).toInt();
            // Add all text before prompt, if exists
            m_output += QStringRef(&line, 0, line.indexOf(m_prompt)).toString();
            if (!expressionQueue().isEmpty())
            {
                const QString& command = expressionQueue().first()->command();
                if (m_previousPromptNumber + 1 == promptNumber || isSpecialOctaveCommand(command))
                {
                    if (!expressionQueue().isEmpty())
                    {
                        readError();
                        static_cast<OctaveExpression*>(expressionQueue().first())->parseOutput(m_output);
                    }
                }
                else
                {
                    // Error command don't increase octave prompt number (usually, but not always)
                    readError();
                }
            }
            m_previousPromptNumber = promptNumber;
            m_output.clear();
        }
        else if (line.contains(m_subprompt) && m_subprompt.cap(1).toInt() == m_previousPromptNumber)
        {
            // User don't write finished octave statement (for example, write 'a = [1,2, ' only), so
            // octave print subprompt and waits input finish.
            m_syntaxError = true;
            qDebug() << "subprompt catch";
            m_process->write(")]'\"\n"); // forse exit from subprompt
            m_output.clear();
        }
        else
            m_output += line;
    }
}

void OctaveSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    qDebug() << "currentExpressionStatusChanged" << status << expressionQueue().first()->command();
    switch (status)
    {
    case Cantor::Expression::Done:
    case Cantor::Expression::Error:
        finishFirstExpression();
        break;

    default:
        break;
    }
}

Cantor::CompletionObject* OctaveSession::completionFor ( const QString& cmd, int index )
{
    return new OctaveCompletionObject ( cmd, index, this );
}

Cantor::SyntaxHelpObject* OctaveSession::syntaxHelpFor ( const QString& cmd )
{
    return new OctaveSyntaxHelpObject ( cmd, this );
}

QSyntaxHighlighter* OctaveSession::syntaxHighlighter ( QObject* parent )
{
    return new OctaveHighlighter ( parent, this );
}

void OctaveSession::runSpecificCommands()
{
    m_process->write("figure(1,'visible','off')");
}

bool OctaveSession::isDoNothingCommand(const QString& command)
{
    return PROMPT_UNCHANGEABLE_COMMAND.exactMatch(command) || command.isEmpty() || command == QLatin1String("\n");
}

bool OctaveSession::isSpecialOctaveCommand(const QString& command)
{
    return command.contains(QLatin1String("completion_matches"));
}
