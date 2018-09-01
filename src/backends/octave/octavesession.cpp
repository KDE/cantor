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
#include "result.h"
#include "textresult.h"

#include "settings.h"
#include "octave-backend-config.h"

#include <KProcess>
#include <KDirWatch>

#include <QTimer>
#include <QFile>
#include "octavehighlighter.h"
#include <settings.h>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

#include <defaultvariablemodel.h>

OctaveSession::OctaveSession ( Cantor::Backend* backend ) : Session ( backend ),
m_process(nullptr),
m_watch(nullptr),
m_variableModel(new Cantor::DefaultVariableModel(this))
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

    // Add the cantor script directory to search path
    args << QLatin1String("--eval");
    args << QString::fromLatin1("addpath %1;").arg(octaveScriptInstallDir);

    if (OctaveSettings::integratePlots())
    {
        // Do not show the popup when plotting, rather only print to a file
        args << QLatin1String("--eval");
        args << QLatin1String("graphics_toolkit gnuplot;");
        args << QLatin1String("--eval");
        args << QLatin1String("set (0, \"defaultfigurevisible\",\"off\");");
    }
    else
    {
        args << QLatin1String("--eval");
        args << QLatin1String("set (0, \"defaultfigurevisible\",\"on\");");
    }

    // Do not show extra text in help commands
    args << QLatin1String("--eval");
    args << QLatin1String("suppress_verbose_help_message(1);");

    // Print the temp dir, used for plot files
    args << QLatin1String("--eval");
    args << QLatin1String("____TMP_DIR____ = tempdir");

    if (OctaveSettings::integratePlots())
    {
        m_watch = new KDirWatch(this);
        m_watch->setObjectName(QLatin1String("OctaveDirWatch"));
        connect (m_watch, SIGNAL(dirty(QString)), SLOT(plotFileChanged(QString)) );
    }

    // connect the signal and slots prior to staring octave to make sure we handle the very first output
    // in parserOutput() to determine the temp folder and the format of the promt
    connect ( m_process, SIGNAL ( readyReadStandardOutput() ), SLOT ( readOutput() ) );
    connect ( m_process, SIGNAL ( readyReadStandardError() ), SLOT ( readError() ) );
    connect ( m_process, SIGNAL ( error ( QProcess::ProcessError ) ), SLOT ( processError() ) );

    m_process->setProgram ( OctaveSettings::path().toLocalFile(), args );
    qDebug() << "starting " << m_process->program();
    m_process->setOutputChannelMode ( KProcess::SeparateChannels );
    m_process->start();
    m_process->waitForStarted();
    m_process->waitForReadyRead();

    if(!OctaveSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = OctaveSettings::self()->autorunScripts().join(QLatin1String("\n"));

        evaluateExpression(autorunScripts, OctaveExpression::DeleteOnFinish);
    }

    emit loginDone();
    qDebug()<<"login done";
}

void OctaveSession::logout()
{
    qDebug()<<"logout";

    if(!m_process)
        return;

    disconnect(m_process, nullptr, this, nullptr);

//     if(status()==Cantor::Session::Running)
        //TODO: terminate the running expressions first

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

    m_prompt = QRegExp();
    m_tempDir.clear();

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
    QString command = expression->command();
    command.replace(QLatin1Char('\n'), QLatin1Char(','));
    command += QLatin1Char('\n');
    expression->setStatus(Cantor::Expression::Computing);
    m_process->write ( command.toLocal8Bit() );
}

void OctaveSession::readError()
{
    qDebug() << "readError";
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (!expressionQueue().isEmpty() && !error.isEmpty())
    {
        static_cast<OctaveExpression*>(expressionQueue().first())->parseError(error);
    }
}

void OctaveSession::readOutput()
{
    qDebug() << "readOutput";
    while (m_process->bytesAvailable() > 0)
    {
        if (m_tempDir.isEmpty() && !m_process->canReadLine())
        {
            qDebug() << "Waiting";
            // Wait for the full line containing octave's tempDir
            return;
        }
        QString line = QString::fromLocal8Bit(m_process->readLine());
        qDebug()<<"start parsing " << "  " << line;
        if (expressionQueue().isEmpty() || m_prompt.isEmpty())
        {
            // no expression is available, we're parsing the first output of octave after the start
            // -> determine the location of the temporary folder and the format of octave's promt
            if (m_prompt.isEmpty() && line.contains(QLatin1String(":1>")))
            {
                qDebug() << "Found Octave prompt:" << line;
                line.replace(QLatin1String(":1"), QLatin1String(":[0-9]+"));
                m_prompt.setPattern(line);
            }
            else if (line.contains(QLatin1String("____TMP_DIR____")))
            {
                m_tempDir = line;
                m_tempDir.remove(0,18);
                m_tempDir.chop(1); // isolate the tempDir's location
                qDebug() << "Got temporary file dir:" << m_tempDir;
                if (m_watch)
                {
                    m_watch->addDir(m_tempDir, KDirWatch::WatchFiles);
                }
            }
        }
        else if (line.contains(m_prompt))
        {
            // Check for errors before finalizing the expression
            // this makes sure that all errors are caught
            readError();
            if (!expressionQueue().isEmpty())
            {
                // Get command before finalize, because after finalizing the expression will be dequeued
                const QString& command = expressionQueue().first()->command();
                static_cast<OctaveExpression*>(expressionQueue().first())->finalize();
                if (command.contains(QLatin1String(" = ")))
                {
                    emit variablesChanged();
                }
                if (command.contains(QLatin1String("function ")))
                {
                    emit functionsChanged();
                }
            }
        }
        else
        {
            // Avoid many calls to setResult if a lot of output came at the same time
            while (m_process->canReadLine())
            {
                line += QString::fromLocal8Bit(m_process->readLine());
            }
            if (!expressionQueue().isEmpty())
                static_cast<OctaveExpression*>(expressionQueue().first())->parseOutput(line);

        }
    }
}

void OctaveSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    qDebug() << "currentExpressionStatusChanged";
    switch (status)
    {
    case Cantor::Expression::Done:
    case Cantor::Expression::Error:
        expressionQueue().removeFirst();
        if (expressionQueue().isEmpty())
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

    connect ( this, SIGNAL(variablesChanged()), highlighter, SLOT(updateVariables()) );

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
