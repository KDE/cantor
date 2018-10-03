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
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#include "pythonsession.h"
#include "pythonexpression.h"
#include "pythonhighlighter.h"
#include "pythoncompletionobject.h"
#include "pythonkeywords.h"
#include "pythonutils.h"

#include <QDebug>
#include <QDir>

#include <QStandardPaths>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include <KProcess>

#include <KDirWatch>

#include <defaultvariablemodel.h>


PythonSession::PythonSession(Cantor::Backend* backend, int pythonVersion, const QString serverName, const QString DbusChannelName)
    : Session(backend)
    , m_variableModel(new Cantor::DefaultVariableModel(this))
    , m_pIface(nullptr)
    , m_pProcess(nullptr)
    , serverName(serverName)
    , DbusChannelName(DbusChannelName)
{
    if (pythonVersion == 2)
        PythonKeywords::instance()->python2Mode();
}

void PythonSession::login()
{
    qDebug()<<"login";
    emit loginStarted();

    // TODO: T6113, T6114
    if (m_pProcess)
        m_pProcess->deleteLater();

    m_pProcess = new KProcess(this);
    m_pProcess->setOutputChannelMode(KProcess::SeparateChannels);

    (*m_pProcess) << QStandardPaths::findExecutable(serverName);

    m_pProcess->start();

    m_pProcess->waitForStarted();
    m_pProcess->waitForReadyRead();
    QTextStream stream(m_pProcess->readAllStandardOutput());

    const QString& readyStatus = QString::fromLatin1("ready");
    while (m_pProcess->state() == QProcess::Running)
    {
        const QString& rl = stream.readLine();
        if (rl == readyStatus)
            break;
    }

    if (!QDBusConnection::sessionBus().isConnected())
    {
        qWarning() << "Can't connect to the D-Bus session bus.\n"
                      "To start it, run: eval `dbus-launch --auto-syntax`";
        return;
    }

    const QString& serviceName = DbusChannelName + QString::fromLatin1("-%1").arg(m_pProcess->pid());

    m_pIface =  new QDBusInterface(serviceName,
                                   QString::fromLatin1("/"), QString(), QDBusConnection::sessionBus());
    if (!m_pIface->isValid())
    {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    m_pIface->call(QString::fromLatin1("login"));
    m_pIface->call(QString::fromLatin1("setFilePath"), worksheetPath);

    if(integratePlots())
    {
        qDebug() << "integratePlots";

        QString tempPath = QDir::tempPath();

        QString pathOperations = tempPath;
        pathOperations.prepend(QLatin1String("import os\nos.chdir('"));
        pathOperations.append(QLatin1String("')\n"));

        qDebug() << "Processing command to change chdir in Python. Command " << pathOperations.toLocal8Bit();

        getPythonCommandOutput(pathOperations);

        m_watch = new KDirWatch(this);
        m_watch->setObjectName(QLatin1String("PythonDirWatch"));

        m_watch->addDir(tempPath, KDirWatch::WatchFiles);

        qDebug() << "addDir " <<  tempPath << "? " << m_watch->contains(QLatin1String(tempPath.toLocal8Bit()));

        QObject::connect(m_watch, SIGNAL(created(QString)), SLOT(plotFileChanged(QString)));
    }

    const QStringList& scripts = autorunScripts();
    if(!scripts.isEmpty()){
        QString autorunScripts = scripts.join(QLatin1String("\n"));
        getPythonCommandOutput(autorunScripts);
    }

    const QString& importerFile = QLatin1String(":py/import_default_modules.py");

    evaluateExpression(fromSource(importerFile), Cantor::Expression::DeleteOnFinish, true);

    listVariables();

    changeStatus(Session::Done);
    emit loginDone();
}

void PythonSession::logout()
{
    // TODO: T6113, T6114
    m_pProcess->terminate();

    qDebug()<<"logout";

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(QLatin1String(i.next().toLocal8Bit().constData()));
    }

    changeStatus(Status::Disable);
}

void PythonSession::interrupt()
{
    // TODO: T6113, T6114
    if (m_pProcess->pid())
        m_pProcess->kill();

    qDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* PythonSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug() << "evaluating: " << cmd;
    PythonExpression* expr = new PythonExpression(this, internal);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void PythonSession::runExpression(PythonExpression* expr)
{
    qDebug() << "run expression";

    m_currentExpression = expr;

    const QString& command = expr->command();
    QStringList commandLine = command.split(QLatin1String("\n"));

    QString commandProcessing;

    for(const QString& command : commandLine){
        const QString firstLineWord = command.trimmed().replace(QLatin1String("("), QLatin1String(" "))
            .split(QLatin1String(" ")).at(0);

        // Ignore comments
        if (firstLineWord.length() != 0 && firstLineWord[0] == QLatin1Char('#')){

            commandProcessing += command + QLatin1String("\n");
            continue;
        }

        if(firstLineWord.contains(QLatin1String("execfile"))){

            commandProcessing += command;
            continue;
        }

        commandProcessing += command + QLatin1String("\n");

    }
    readExpressionOutput(commandProcessing);
}

// Is called asynchronously in the Python3 plugin
void PythonSession::readExpressionOutput(const QString& commandProcessing)
{
    readOutput(commandProcessing);
}

void PythonSession::getPythonCommandOutput(const QString& commandProcessing)
{
    qDebug() << "Running python command" << commandProcessing;

    runPythonCommand(commandProcessing);

    m_output = getOutput();
    m_error = getError();
}

bool PythonSession::identifyKeywords(const QString& command)
{
    QString verifyErrorImport;

    QString listKeywords;
    QString keywordsString;

    QString moduleImported;
    QString moduleVariable;

    getPythonCommandOutput(command);

    qDebug() << "verifyErrorImport: ";

    if(!m_error.isEmpty()){

        qDebug() << "returned false";

        return false;
    }

    moduleImported += identifyPythonModule(command);
    moduleVariable += identifyVariableModule(command);

    if((moduleVariable.isEmpty()) && (!command.endsWith(QLatin1String("*")))){
        keywordsString = command.section(QLatin1String(" "), 3).remove(QLatin1String(" "));
    }

    if(moduleVariable.isEmpty() && (command.endsWith(QLatin1String("*")))){
        listKeywords += QString::fromLatin1("import %1\n"     \
                                            "print(dir(%1))\n" \
                                            "del %1\n").arg(moduleImported);
    }

    if(!moduleVariable.isEmpty()){
        listKeywords += QLatin1String("print(dir(") + moduleVariable + QLatin1String("))\n");
    }

    if(!listKeywords.isEmpty()){
        getPythonCommandOutput(listKeywords);

        keywordsString = m_output;

        keywordsString.remove(QLatin1String("'"));
        keywordsString.remove(QLatin1String(" "));
        keywordsString.remove(QLatin1String("["));
        keywordsString.remove(QLatin1String("]"));
    }

    QStringList keywordsList = keywordsString.split(QLatin1String(","));
    PythonKeywords::instance()->loadFromModule(moduleVariable, keywordsList);

    qDebug() << "Module imported" << moduleImported;

    return true;
}

QString PythonSession::identifyPythonModule(const QString& command) const
{
    QString module;

    if(command.contains(QLatin1String("import "))){
        module = command.section(QLatin1String(" "), 1, 1);
    }

    qDebug() << "module identified" << module;
    return module;
}

QString PythonSession::identifyVariableModule(const QString& command) const
{
    QString variable;

    if(command.contains(QLatin1String("import "))){
        variable = command.section(QLatin1String(" "), 1, 1);
    }

    if((command.contains(QLatin1String("import "))) && (command.contains(QLatin1String(" as ")))){
        variable = command.section(QLatin1String(" "), 3, 3);
    }

    if(command.contains(QLatin1String("from "))){
        variable = QLatin1String("");
    }

    qDebug() << "variable identified" << variable;
    return variable;
}

void PythonSession::expressionFinished()
{
    qDebug()<< "finished";
    PythonExpression* expression = qobject_cast<PythonExpression*>(sender());

    m_runningExpressions.removeAll(expression);
    qDebug() << "size: " << m_runningExpressions.size();
}

void PythonSession::updateOutput()
{
    if(m_error.isEmpty()){
        m_currentExpression->parseOutput(m_output);

        qDebug() << "output: " << m_output;
    } else {
        m_currentExpression->parseError(m_error);

        qDebug() << "error: " << m_error;
    }

    listVariables();

    changeStatus(Cantor::Session::Done);
}

void PythonSession::readOutput(const QString& commandProcessing)
{
    qDebug() << "readOutput";

    getPythonCommandOutput(commandProcessing);

    updateOutput();
}

void PythonSession::plotFileChanged(const QString& filename)
{
    qDebug() << "plotFileChanged filename:" << filename;

    if ((m_currentExpression) && (filename.contains(QLatin1String("cantor-export-python-figure"))))
    {
         qDebug() << "Calling parsePlotFile";
         m_currentExpression->parsePlotFile(filename);

         m_listPlotName.append(filename);
    }
}

void PythonSession::listVariables()
{
    const QString& listVariableCommand = QLatin1String(
        "try: \n"
        "   import numpy \n"
        "   __cantor_numpy_internal__ = numpy.get_printoptions()['threshold'] \n"
        "   numpy.set_printoptions(threshold=100000000) \n"
        "except ModuleNotFoundError: \n"
        "   pass \n"

        "print(globals()) \n"

        "try: \n"
        "   import numpy \n"
        "   numpy.set_printoptions(threshold=__cantor_numpy_internal__) \n"
        "   del __cantor_numpy_internal__ \n"
        "except ModuleNotFoundError: \n"
        "   pass \n"
    );

    getPythonCommandOutput(listVariableCommand);

    qDebug() << m_output;

    m_output.remove(QLatin1String("{"));
    m_output.remove(QLatin1String("<"));
    m_output.remove(QLatin1String(">"));
    m_output.remove(QLatin1String("}"));

    foreach(QString line, m_output.split(QLatin1String(", '"))){

        QStringList parts = line.simplified().split(QLatin1String(":"));
        const QString& first = parts.first();
        const QString& last = parts.last();
        if(!first.startsWith(QLatin1String("'__")) && !first.startsWith(QLatin1String("__")) && !first.startsWith(QLatin1String("CatchOutPythonBackend'")) &&
           !first.startsWith(QLatin1String("errorPythonBackend'")) && !first.startsWith(QLatin1String("outputPythonBackend'")) &&
           !last.startsWith(QLatin1String(" class ")) && !last.startsWith(QLatin1String(" function ")) &&
           !last.startsWith(QLatin1String(" module '")) /*skip imported modules*/ )
        {

            m_variableModel->addVariable(parts.first().remove(QLatin1String("'")).simplified(), parts.last().simplified());
            emit newVariable(parts.first().remove(QLatin1String("'")).simplified());
        }
    }

    emit updateHighlighter();
}

QSyntaxHighlighter* PythonSession::syntaxHighlighter(QObject* parent)
{
    PythonHighlighter* highlighter = new PythonHighlighter(parent);
    QObject::connect(this, SIGNAL(updateHighlighter()), highlighter, SLOT(updateHighlight()));
    QObject::connect(this, SIGNAL(newVariable(QString)), highlighter, SLOT(addVariable(QString)));

    return highlighter;
}

Cantor::CompletionObject* PythonSession::completionFor(const QString& command, int index)
{
    return new PythonCompletionObject(command, index, this);
}

QAbstractItemModel* PythonSession::variableModel()
{
    return m_variableModel;
}

void PythonSession::runPythonCommand(const QString& command) const
{
    // TODO: T6113, T6114
    m_pIface->call(QString::fromLatin1("runPythonCommand"), command);
}

QString PythonSession::getOutput() const
{
    // TODO: T6113, T6114
    const QDBusReply<QString>& reply = m_pIface->call(QString::fromLatin1("getOutput"));
    if (reply.isValid())
        return reply.value();

    return reply.error().message();
}

QString PythonSession::getError() const
{
    // TODO: T6113, T6114
    const QDBusReply<QString>& reply = m_pIface->call(QString::fromLatin1("getError"));
    if (reply.isValid())
        return reply.value();

    return reply.error().message();
}

void PythonSession::setWorksheetPath(const QString& path)
{
    worksheetPath = path;
}
