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
#include "scilabkeywords.h"
#include "scilabcompletionobject.h"

#include <defaultvariablemodel.h>

#include <KProcess>
#include <KDirWatch>

#include <QDebug>
#include <QDir>
#include <QtCore/QFile>
#include <QTextEdit>
#include <QListIterator>
#include <QIODevice>
#include <QByteArray>

#include <settings.h>

ScilabSession::ScilabSession( Cantor::Backend* backend) : Session(backend),
m_variableModel(new Cantor::DefaultVariableModel(this))
{
    m_process = nullptr;
    qDebug();
}

ScilabSession::~ScilabSession()
{
    m_process->terminate();
    qDebug();
}

void ScilabSession::login()
{
    qDebug()<<"login";
    emit loginStarted();

    QStringList args;

    args << QLatin1String("-nb");

    m_process = new KProcess(this);
    m_process->setProgram(ScilabSettings::self()->path().toLocalFile(), args);

    qDebug() << m_process->program();

    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->start();

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

    QObject::connect(m_process, &KProcess::readyReadStandardOutput, this, &ScilabSession::listKeywords);
    QObject::connect(m_process, &KProcess::readyReadStandardError, this, &ScilabSession::readError);

    m_process->readAllStandardOutput().clear();
    m_process->readAllStandardError().clear();

    ScilabExpression *listKeywords = new ScilabExpression(this);
    listKeywords->setCommand(QLatin1String("disp(getscilabkeywords());"));
    runExpression(listKeywords);

    emit loginDone();
}

void ScilabSession::logout()
{
    qDebug()<<"logout";

    m_process->write("exit\n");

    m_runningExpressions.clear();
    qDebug() << "m_runningExpressions: " << m_runningExpressions.isEmpty();

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(QLatin1String(i.next().toLocal8Bit().constData()));
    }
}

void ScilabSession::interrupt()
{
    qDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* ScilabSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    qDebug() << "evaluating: " << cmd;
    ScilabExpression* expr = new ScilabExpression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void ScilabSession::runExpression(ScilabExpression* expr)
{
    QString command;

    command.prepend(QLatin1String("\nprintf('begin-cantor-scilab-command-processing')\n"));
    command += expr->command();

    m_currentExpression = expr;

    connect(m_currentExpression, &ScilabExpression::statusChanged, this, &ScilabSession::currentExpressionStatusChanged);

    command += QLatin1String("\nprintf('terminated-cantor-scilab-command-processing')\n");
    qDebug() << "Writing command to process" << command;

    m_process->write(command.toLocal8Bit());
}

void ScilabSession::expressionFinished()
{
    qDebug()<<"finished";
    ScilabExpression* expression = qobject_cast<ScilabExpression*>(sender());

    m_runningExpressions.removeAll(expression);
    qDebug() << "size: " << m_runningExpressions.size();
}

void ScilabSession::readError()
{
    qDebug() << "readError";

    QString error = QLatin1String(m_process->readAllStandardError());

    qDebug() << "error: " << error;
    m_currentExpression->parseError(error);
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

        m_currentExpression->parseOutput(m_output);

        m_output.clear();

    }
}

void ScilabSession::plotFileChanged(QString filename)
{
    qDebug() << "plotFileChanged filename:" << filename;

    if ((m_currentExpression) && (filename.contains(QLatin1String("cantor-export-scilab-figure")))){
         qDebug() << "Calling parsePlotFile";
         m_currentExpression->parsePlotFile(filename);

         m_listPlotName.append(filename);
    }
}

void ScilabSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    qDebug() << "currentExpressionStatusChanged: " << status;

    switch (status){
        case Cantor::Expression::Computing:
            break;

        case Cantor::Expression::Interrupted:
            break;

        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            changeStatus(Done);

            emit updateVariableHighlighter();

            break;
    }
}

void ScilabSession::listKeywords()
{
    qDebug();

    while(m_process->bytesAvailable() > 0){
        m_output.append(QString::fromLocal8Bit(m_process->readLine()));
    }

    if(m_output.contains(QLatin1String("begin-cantor-scilab-command-processing")) &&
        m_output.contains(QLatin1String("terminated-cantor-scilab-command-processing"))){

        m_output.remove(QLatin1String("begin-cantor-scilab-command-processing"));
        m_output.remove(QLatin1String("terminated-cantor-scilab-command-processing"));

        ScilabKeywords::instance()->setupKeywords(m_output);

        QObject::disconnect(m_process, &KProcess::readyReadStandardOutput, this, &ScilabSession::listKeywords);
        QObject::connect(m_process, &KProcess::readyReadStandardOutput, this, &ScilabSession::readOutput);

        m_process->readAllStandardOutput().clear();
        m_process->readAllStandardError().clear();

        m_output.clear();
    }

    changeStatus(Done);
    m_currentExpression->evalFinished();

    emit updateHighlighter();
}

QSyntaxHighlighter* ScilabSession::syntaxHighlighter(QObject* parent)
{
    qDebug();

    ScilabHighlighter *highlighter = new ScilabHighlighter(parent);

    QObject::connect(this, &ScilabSession::updateHighlighter, highlighter, &ScilabHighlighter::updateHighlight);
    QObject::connect(this, &ScilabSession::updateVariableHighlighter, highlighter, &ScilabHighlighter::addVariableHighlight);
    return highlighter;
}

Cantor::CompletionObject* ScilabSession::completionFor(const QString& command, int index)
{
    return new ScilabCompletionObject(command, index, this);
}

QAbstractItemModel* ScilabSession::variableModel()
{
    return m_variableModel;
}

