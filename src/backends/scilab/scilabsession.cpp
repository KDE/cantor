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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "scilabsession.h"
#include "scilabexpression.h"
#include "scilabhighlighter.h"
#include "scilabcompletionobject.h"

#include <defaultvariablemodel.h>

#include <QProcess>
#include <KDirWatch>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextEdit>
#include <QListIterator>
#include <QIODevice>
#include <QByteArray>

#include <settings.h>

ScilabSession::ScilabSession( Cantor::Backend* backend) : Session(backend),
m_process(nullptr),
m_watch(nullptr),
m_variableModel(new Cantor::DefaultVariableModel(this))
{
}

ScilabSession::~ScilabSession()
{
    if (m_process)
        m_process->terminate();
}

void ScilabSession::login()
{
    qDebug()<<"login";
    emit loginStarted();

    QStringList args;

    args << QLatin1String("-nb");

    m_process = new QProcess(this);
    m_process->setArguments(args);
    m_process->setProgram(ScilabSettings::self()->path().toLocalFile());

    qDebug() << m_process->program();

    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    m_process->start();
    m_process->waitForStarted();

    if(ScilabSettings::integratePlots()){

        qDebug() << "integratePlots";

        QString tempPath = QDir::tempPath();

        QString pathScilabOperations = tempPath;
        pathScilabOperations.prepend(QLatin1String("chdir('"));
        pathScilabOperations.append(QLatin1String("');\n"));

        qDebug() << "Processing command to change chdir in Scilab. Command " << pathScilabOperations.toLocal8Bit();

        m_process->write(pathScilabOperations.toLocal8Bit());

        m_watch = new KDirWatch(this);
        m_watch->setObjectName(QLatin1String("ScilabDirWatch"));

        m_watch->addDir(tempPath, KDirWatch::WatchFiles);

        qDebug() << "addDir " <<  tempPath << "? " << m_watch->contains(QLatin1String(tempPath.toLocal8Bit()));

        QObject::connect(m_watch, &KDirWatch::created, this, &ScilabSession::plotFileChanged);
    }

    if(!ScilabSettings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = ScilabSettings::self()->autorunScripts().join(QLatin1String("\n"));
        m_process->write(autorunScripts.toLocal8Bit());
    }

    QObject::connect(m_process, &QProcess::readyReadStandardOutput, this, &ScilabSession::readOutput);
    QObject::connect(m_process, &QProcess::readyReadStandardError, this, &ScilabSession::readError);

    m_process->readAllStandardOutput().clear();
    m_process->readAllStandardError().clear();

    changeStatus(Cantor::Session::Done);

    emit loginDone();
}

void ScilabSession::logout()
{
    qDebug()<<"logout";

    m_process->write("exit\n");

    expressionQueue().clear();
    m_variableModel->clearVariables();

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(QLatin1String(i.next().toLocal8Bit().constData()));
    }

    changeStatus(Status::Disable);
}

void ScilabSession::interrupt()
{
    qDebug()<<"interrupt";

    if (status() == Cantor::Session::Running)
        expressionQueue().first()->interrupt();

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* ScilabSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug() << "evaluating: " << cmd;
    ScilabExpression* expr = new ScilabExpression(this, internal);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void ScilabSession::runFirstExpression()
{
    qDebug() <<"call runFirstExpression";
    qDebug() << "m_process: " << m_process;
    qDebug() << "status: " << (status() == Cantor::Session::Running ? "Running" : "Done");

    if (!m_process)
        return;

    qDebug()<<"running next expression";

    if(!expressionQueue().isEmpty())
    {
        ScilabExpression* expr = static_cast<ScilabExpression*>(expressionQueue().first());

        QString command;
        command.prepend(QLatin1String("\nprintf('begin-cantor-scilab-command-processing')\n"));
        command += expr->command();
        command += QLatin1String("\nprintf('terminated-cantor-scilab-command-processing')\n");

        connect(expr, &ScilabExpression::statusChanged, this, &ScilabSession::currentExpressionStatusChanged);
        expr->setStatus(Cantor::Expression::Computing);

        qDebug() << "Writing command to process" << command;

        m_process->write(command.toLocal8Bit());
    }
}

void ScilabSession::readError()
{
    qDebug() << "readError";

    QString error = QLatin1String(m_process->readAllStandardError());

    qDebug() << "error: " << error;
    if (!expressionQueue().isEmpty())
        static_cast<ScilabExpression*>(expressionQueue().first())->parseError(error);
}

void ScilabSession::readOutput()
{
    qDebug() << "readOutput";

    while(m_process->bytesAvailable() > 0){
        m_output.append(QString::fromLocal8Bit(m_process->readLine()));
    }

    qDebug() << "output.isNull? " << m_output.isNull();
    qDebug() << "output: " << m_output;

    if(status() != Running || m_output.isNull()){
        return;
    }

    if(m_output.contains(QLatin1String("begin-cantor-scilab-command-processing")) &&
        m_output.contains(QLatin1String("terminated-cantor-scilab-command-processing"))){

        m_output.remove(QLatin1String("begin-cantor-scilab-command-processing"));
        m_output.remove(QLatin1String("terminated-cantor-scilab-command-processing"));

        static_cast<ScilabExpression*>(expressionQueue().first())->parseOutput(m_output);

        m_output.clear();
    }
}

void ScilabSession::plotFileChanged(const QString& filename)
{
    qDebug() << "plotFileChanged filename:" << filename;

    if (!expressionQueue().isEmpty() && (filename.contains(QLatin1String("cantor-export-scilab-figure")))){
         qDebug() << "Calling parsePlotFile";
         static_cast<ScilabExpression*>(expressionQueue().first())->parsePlotFile(filename);

         m_listPlotName.append(filename);
    }
}

void ScilabSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    qDebug() << "currentExpressionStatusChanged: " << status;

    switch (status){
        case Cantor::Expression::Computing:
        case Cantor::Expression::Interrupted:
        case Cantor::Expression::Queued:
            break;

        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            expressionQueue().removeFirst();

            if (expressionQueue().isEmpty())
                changeStatus(Done);
            else
                runFirstExpression();

            break;
    }
}

QSyntaxHighlighter* ScilabSession::syntaxHighlighter(QObject* parent)
{

    ScilabHighlighter *highlighter = new ScilabHighlighter(parent, this);

    return highlighter;
}

Cantor::CompletionObject* ScilabSession::completionFor(const QString& command, int index)
{
    return new ScilabCompletionObject(command, index, this);
}

Cantor::DefaultVariableModel* ScilabSession::variableModel() const
{
    return m_variableModel;
}

