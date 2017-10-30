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

#include <signal.h>
#include <defaultvariablemodel.h>

OctaveSession::OctaveSession ( Cantor::Backend* backend ) : Session ( backend ),
m_process(0),
m_currentExpression(0),
m_watch(0),
m_variableModel(new Cantor::DefaultVariableModel(this))
{
    qDebug() << octaveScriptInstallDir;
}

OctaveSession::~OctaveSession()
{

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

    m_process->setProgram ( OctaveSettings::path().toLocalFile(), args );
    qDebug() << m_process->program();
    m_process->setOutputChannelMode ( KProcess::SeparateChannels );
    connect ( m_process, SIGNAL ( readyReadStandardOutput() ), SLOT ( readOutput() ) );
    connect ( m_process, SIGNAL ( readyReadStandardError() ), SLOT ( readError() ) );
    connect ( m_process, SIGNAL ( error ( QProcess::ProcessError ) ), SLOT ( processError() ) );
    m_process->start();

    if (OctaveSettings::integratePlots())
    {
        m_watch = new KDirWatch(this);
        m_watch->setObjectName(QLatin1String("OctaveDirWatch"));
        connect (m_watch, SIGNAL(dirty(QString)), SLOT(plotFileChanged(QString)) );
    }

    if(!OctaveSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = OctaveSettings::self()->autorunScripts().join(QLatin1String("\n"));

        evaluateExpression(autorunScripts, OctaveExpression::DeleteOnFinish);
    }
}

void OctaveSession::logout()
{
    qDebug() << "logout";
    m_process->write("exit\n");
    if (!m_process->waitForFinished(1000))
    {
        m_process->kill();
    }
}

void OctaveSession::interrupt()
{
    qDebug() << "interrupt";
    if (m_currentExpression)
    {
        m_currentExpression->interrupt();
    }
    m_expressionQueue.clear();
    qDebug() << "Sending SIGINT to Octave";
    kill(m_process->pid(), SIGINT);
    changeStatus(Done);
}

void OctaveSession::processError()
{
    qDebug() << "processError";
    emit error(m_process->errorString());
}

Cantor::Expression* OctaveSession::evaluateExpression ( const QString& command, Cantor::Expression::FinishingBehavior finishingBehavior )
{
    qDebug() << "evaluateExpression: " << command;
    OctaveExpression* expression = new OctaveExpression ( this );
    expression->setCommand ( command );
    expression->setFinishingBehavior ( finishingBehavior );
    expression->evaluate();

    return expression;
}

void OctaveSession::runExpression ( OctaveExpression* expression )
{
    qDebug() << "runExpression";
    if ( status() != Done ) {
        m_expressionQueue.enqueue ( expression );
        qDebug() << m_expressionQueue.size();
    } else {
        m_currentExpression = expression;
        changeStatus(Running);
        connect(m_currentExpression, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(currentExpressionStatusChanged(Cantor::Expression::Status)));
        QString command = expression->command();
        command.replace(QLatin1Char('\n'), QLatin1Char(','));
        command += QLatin1Char('\n');
        m_process->write ( command.toLocal8Bit() );
    }
}

void OctaveSession::readError()
{
    qDebug() << "readError";
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (!m_currentExpression || error.isEmpty())
    {
        return;
    }
    m_currentExpression->parseError(error);
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
        if (!m_currentExpression)
        {
            if (m_prompt.isEmpty() && line.contains(QLatin1String(":1>")))
            {
                qDebug() << "Found Octave prompt:" << line;
                line.replace(QLatin1String(":1"), QLatin1String(":[0-9]+"));
                m_prompt.setPattern(line);
                changeStatus(Done);
                if (!m_expressionQueue.isEmpty())
                {
                    runExpression(m_expressionQueue.dequeue());
                }
                emit loginDone();
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
            m_currentExpression->finalize();
            if (m_currentExpression->command().contains(QLatin1String(" = ")))
            {
                emit variablesChanged();
            }
            if (m_currentExpression->command().contains(QLatin1String("function ")))
            {
                emit functionsChanged();
            }
        }
        else
        {
            // Avoid many calls to setResult if a lot of output came at the same time
            while (m_process->canReadLine())
            {
                line += QString::fromLocal8Bit(m_process->readLine());
            }
            m_currentExpression->parseOutput(line);

        }
    }
}

void OctaveSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    qDebug() << "currentExpressionStatusChanged";
    if (!m_currentExpression)
    {
        return;
    }
    switch (status)
    {
    case Cantor::Expression::Computing:
        break;
    case Cantor::Expression::Interrupted:
        break;
    case Cantor::Expression::Done:
    case Cantor::Expression::Error:
        changeStatus(Done);
        if (!m_expressionQueue.isEmpty())
        {
            runExpression(m_expressionQueue.dequeue());
        }
        break;
    }
}

void OctaveSession::plotFileChanged(const QString& filename)
{
    if (!QFile::exists(filename) || !filename.split(QLatin1Char('/')).last().contains(QLatin1String("c-ob-")))
    {
        return;
    }
    if (m_currentExpression)
    {
        m_currentExpression->parsePlotFile(filename);
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
    connect ( this, SIGNAL(functionsChanged()), highlighter, SLOT(updateFunctions()) );
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
