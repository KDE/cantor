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
#include "octave-backend-config.h"
#include "octavehighlighter.h"

#include <KProcess>
#include <KDirWatch>
#include <KLocalizedString>

#include <QTimer>
#include <QFile>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

#include "octavevariablemodel.h"

const QRegExp OctaveSession::PROMPT_UNCHANGEABLE_COMMAND = QRegExp(QLatin1String("(,|;)+"));

OctaveSession::OctaveSession ( Cantor::Backend* backend ) : Session ( backend ),
m_process(nullptr),
m_prompt(QLatin1String("CANTOR_OCTAVE_BACKEND_PROMPT:([0-9]+)> ")),
m_subprompt(QLatin1String("CANTOR_OCTAVE_BACKEND_SUBPROMPT:([0-9]+)> ")),
m_previousPromptNumber(1),
m_watch(nullptr),
m_syntaxError(false),
m_needUpdate(false),
m_variableModel(new OctaveVariableModel(this))
{
    qDebug() << octaveScriptInstallDir;
}

void OctaveSession::login()
{
    qDebug() << "login";
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

    // Add the cantor script directory to search path
    args << QLatin1String("--eval");
    args << QString::fromLatin1("addpath %1;").arg(octaveScriptInstallDir);

    // Print the temp dir, used for plot files
    args << QLatin1String("--eval");
    args << QLatin1String("printf('%s\\n', ['____TMP_DIR____ = ' tempdir]);");

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

    if (OctaveSettings::integratePlots())
    {
        m_watch = new KDirWatch(this);
        m_watch->setObjectName(QLatin1String("OctaveDirWatch"));
        connect (m_watch, SIGNAL(dirty(QString)), SLOT(plotFileChanged(QString)) );
    }

    m_process->setProgram ( OctaveSettings::path().toLocalFile(), args );
    qDebug() << "starting " << m_process->program();
    m_process->setOutputChannelMode ( KProcess::SeparateChannels );
    m_process->start();
    m_process->waitForStarted();

    // Got tmp dir
    bool loginFinished = false;
    QString input;
    while (!loginFinished)
    {
        m_process->waitForReadyRead();
        input += QString::fromLatin1(m_process->readAllStandardOutput());
        qDebug() << "login input: " << input;
        if (input.contains(QLatin1String("____TMP_DIR____")))
            {
                m_tempDir = input;
                m_tempDir.remove(0,18);
                m_tempDir.chop(1); // isolate the tempDir's location
                qDebug() << "Got temporary file dir:" << m_tempDir;
                if (m_watch)
                {
                    m_watch->addDir(m_tempDir, KDirWatch::WatchFiles);
                }
                loginFinished = true;
            }
    }

    connect ( m_process, SIGNAL (readyReadStandardOutput()), SLOT (readOutput()) );
    connect ( m_process, SIGNAL (readyReadStandardError()), SLOT (readError()) );
    connect ( m_process, SIGNAL (error(QProcess::ProcessError)), SLOT (processError()) );

    if(!OctaveSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = OctaveSettings::self()->autorunScripts().join(QLatin1String("\n"));

        evaluateExpression(autorunScripts, OctaveExpression::DeleteOnFinish, true);
        m_needUpdate = true;
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

    // if(status()==Cantor::Session::Running)
    // TODO: terminate the running expressions first

    m_process->write("exit\n");
    qDebug()<<"waiting for octave to finish";
    m_process->waitForFinished();
    qDebug()<<"octave exit finished";

    if(m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        qDebug()<<"octave still running, process kill enforced";
    }

    expressionQueue().clear();
    delete m_process;
    m_process = nullptr;


    m_tempDir.clear();
    m_output.clear();
    m_previousPromptNumber = 1;

    m_variableModel->clearVariables();

    changeStatus(Status::Disable);

    qDebug()<<"logout done";
}

void OctaveSession::interrupt()
{
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
        expressionQueue().first()->interrupt();
        expressionQueue().removeFirst();

        foreach (Cantor::Expression* expression, expressionQueue())
            expression->setStatus(Cantor::Expression::Done);
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
            if (!expressionQueue().isEmpty())
            {
                const QString& command = expressionQueue().first()->command();
                if (m_previousPromptNumber + 1 == promptNumber || isSpecialOctaveCommand(command))
                {
                    if (!expressionQueue().isEmpty())
                        static_cast<OctaveExpression*>(expressionQueue().first())->parseOutput(m_output);
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
    qDebug() << "currentExpressionStatusChanged";
    switch (status)
    {
    case Cantor::Expression::Done:
    case Cantor::Expression::Error:
        m_needUpdate |= !expressionQueue().first()->isInternal();
        expressionQueue().removeFirst();
        if (expressionQueue().isEmpty())
            if (m_needUpdate)
            {
                m_variableModel->update();
                m_needUpdate = false;
            }
            else
                changeStatus(Done);
        else
            runFirstExpression();
        break;
    default:
        break;
    }
}

void OctaveSession::plotFileChanged(const QString& filename)
{
    if (!QFile::exists(filename) || !filename.split(QLatin1Char('/')).last().contains(QLatin1String("c-ob-")))
    {
        return;
    }

    if (!expressionQueue().isEmpty())
    {
        static_cast<OctaveExpression*>(expressionQueue().first())->parsePlotFile(filename);
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
    OctaveHighlighter* highlighter = new OctaveHighlighter ( parent, this );

    connect ( m_variableModel, &Cantor::DefaultVariableModel::variablesAdded, highlighter, &OctaveHighlighter::addUserVariable);
    connect ( m_variableModel, &Cantor::DefaultVariableModel::variablesRemoved, highlighter, &OctaveHighlighter::removeUserVariable);

    return highlighter;
}

QAbstractItemModel* OctaveSession::variableModel()
{
    return m_variableModel;
}


void OctaveSession::runSpecificCommands()
{
    m_process->write("figure(1,'visible','off')");
}

bool OctaveSession::isDoNothingCommand(const QString& command)
{
    return PROMPT_UNCHANGEABLE_COMMAND.exactMatch(command) || command.isEmpty();
}

bool OctaveSession::isSpecialOctaveCommand(const QString& command)
{
    return command.contains(QLatin1String("completion_matches"));
}
