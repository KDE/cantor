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
 */

#include "python2session.h"
#include "python2expression.h"
#include "python2highlighter.h"
#include "python2completionobject.h"
#include "python2keywords.h"

#include <kdebug.h>
#include <KDirWatch>

#include <QtCore/QFile>
#include <QTextEdit>
#include <QListIterator>
#include <QDir>
#include <QIODevice>
#include <QByteArray>
#include <QStringList>

#include <settings.h>
#include <defaultvariablemodel.h>
#include <qdir.h>

#include <string>

using namespace std;

Python2Session::Python2Session(Cantor::Backend* backend) : Session(backend),
m_variableModel(new Cantor::DefaultVariableModel(this))
{
    kDebug();
}

Python2Session::~Python2Session()
{
    kDebug();
}

void Python2Session::login()
{
    kDebug()<<"login";

    Py_Initialize();
    m_pModule = PyImport_AddModule("__main__");

    if(Python2Settings::integratePlots())
    {
        kDebug() << "integratePlots";

        QString tempPath = QDir::tempPath();

        QString pathOperations = tempPath;
        pathOperations.prepend("import os\nos.chdir('");
        pathOperations.append("')\n");

        kDebug() << "Processing command to change chdir in Python. Command " << pathOperations.toLocal8Bit();

        getPythonCommandOutput(pathOperations);

        m_watch = new KDirWatch(this);
        m_watch->setObjectName("PythonDirWatch");

        m_watch->addDir(tempPath, KDirWatch::WatchFiles);

        kDebug() << "addDir " <<  tempPath << "? " << m_watch->contains(tempPath.toLocal8Bit());

        QObject::connect(m_watch, SIGNAL(created(QString)), SLOT(plotFileChanged(QString)));
    }

    if(!Python2Settings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = Python2Settings::self()->autorunScripts().join("\n");
        getPythonCommandOutput(autorunScripts);
    }

    listVariables();

    emit ready();
}

void Python2Session::logout()
{
    kDebug()<<"logout";

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(i.next().toLocal8Bit().constData());
    }

    changeStatus(Cantor::Session::Done);
}

void Python2Session::interrupt()
{
    kDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* Python2Session::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug() << "evaluating: " << cmd;
    Python2Expression* expr = new Python2Expression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void Python2Session::runExpression(Python2Expression* expr)
{
    kDebug() << "run expression";

    m_currentExpression = expr;

    QString command;

    command += expr->command();

    QStringList commandLine = command.split("\n");

    QString commandProcessing;

    for(int contLine = 0; contLine < commandLine.size(); contLine++){

        QString firstLineWord = commandLine.at(contLine).trimmed().replace("(", " ").split(" ").at(0);

        if(commandLine.at(contLine).contains("import ")){

            if(identifyKeywords(commandLine.at(contLine).simplified())){
                continue;
            } else {
                readOutput(expr, commandLine.at(contLine).simplified());
                return;
            }
        }

        if(firstLineWord.contains("execfile")){

            commandProcessing += commandLine.at(contLine);
            continue;
        }

        if((!Python2Keywords::instance()->keywords().contains(firstLineWord)) && (!commandLine.at(contLine).contains("=")) &&
           (!commandLine.at(contLine).endsWith(":")) && (!commandLine.at(contLine).startsWith(" "))){

            commandProcessing += "print " + commandLine.at(contLine) + "\n";

            continue;
        }

        if(commandLine.at(contLine).startsWith(" ")){

            if((Python2Keywords::instance()->keywords().contains(firstLineWord)) || (commandLine.at(contLine).contains("=")) ||
               (commandLine.at(contLine).endsWith(":"))){

                commandProcessing += commandLine.at(contLine) + "\n";

                continue;
            }

            int contIdentationSpace;

            for(contIdentationSpace = 0; commandLine.at(contLine).at(contIdentationSpace).isSpace(); contIdentationSpace++);

            kDebug() << "contIdentationSpace: " << contIdentationSpace;

            QString commandIdentation;

            commandIdentation = commandLine.at(contLine);

            kDebug() << "Insert print in " << contIdentationSpace << "space";
            kDebug() << "commandIdentation before insert " << commandIdentation;

            commandIdentation.insert(contIdentationSpace, QString("print "));

            kDebug() << "commandIdentation after insert" << commandIdentation;

            commandProcessing += commandIdentation + "\n";

            continue;
        }

        commandProcessing += commandLine.at(contLine) + "\n";

    }

    readOutput(expr, commandProcessing);
}

void Python2Session::runClassOutputPython()
{
    QString classOutputPython = "import sys\n"                                      \
                                "class CatchOutPythonBackend:\n"                    \
                                "    def __init__(self):\n"                         \
                                "        self.value = ''\n"                         \
                                "    def write(self, txt):\n"                       \
                                "        self.value += txt\n"                       \
                                "outputPythonBackend = CatchOutPythonBackend()\n"   \
                                "errorPythonBackend  = CatchOutPythonBackend()\n"   \
                                "sys.stdout = outputPythonBackend\n"   \
                                "sys.stderr = errorPythonBackend\n";

    PyRun_SimpleString(classOutputPython.toStdString().c_str());
}

void Python2Session::getPythonCommandOutput(QString commandProcessing)
{
    kDebug() << "Running python command" << commandProcessing.toStdString().c_str();

    runClassOutputPython();
    PyRun_SimpleString(commandProcessing.toStdString().c_str());

    PyObject *outputPython = PyObject_GetAttrString(m_pModule, "outputPythonBackend");
    PyObject *output = PyObject_GetAttrString(outputPython, "value");
    string outputString = PyString_AsString(output);

    PyObject *errorPython = PyObject_GetAttrString(m_pModule, "errorPythonBackend");
    PyObject *error = PyObject_GetAttrString(errorPython, "value");
    string errorString = PyString_AsString(error);

    m_output = QString(outputString.c_str());

    m_error = QString(errorString.c_str());
}

bool Python2Session::identifyKeywords(QString command)
{
    QString verifyErrorImport;

    QString listKeywords;
    QString keywordsString;

    QString moduleImported;
    QString moduleVariable;

    getPythonCommandOutput(command);

    kDebug() << "verifyErrorImport: ";

    if(!m_error.isEmpty()){

        kDebug() << "returned false";

        return false;
    }

    moduleImported += identifyPythonModule(command);
    moduleVariable += identifyVariableModule(command);

    if((moduleVariable.isEmpty()) && (!command.endsWith("*"))){
        keywordsString = command.section(" ", 3).remove(" ");
    }

    if(moduleVariable.isEmpty() && (command.endsWith("*"))){
        listKeywords += "import " + moduleImported + "\n"     \
                        "print dir(" + moduleImported + ")\n" \
                        "del " + moduleImported + "\n";
    }

    if(!moduleVariable.isEmpty()){
        listKeywords += "print dir(" + moduleVariable + ")\n";
    }

    if(!listKeywords.isEmpty()){
        getPythonCommandOutput(listKeywords);

        keywordsString = m_output;

        keywordsString.remove("'");
        keywordsString.remove(" ");
        keywordsString.remove("[");
        keywordsString.remove("]");
    }

    kDebug() << "keywordsString" << keywordsString;

    QStringList keywordsList = keywordsString.split(",");

    kDebug() << "keywordsList" << keywordsList;

    Python2Keywords::instance()->loadFromModule(moduleVariable, keywordsList);

    kDebug() << "Module imported" << moduleImported;

    return true;
}

QString Python2Session::identifyPythonModule(QString command)
{
    QString module;

    if(command.contains("import ")){
        module = command.section(" ", 1, 1);
    }

    kDebug() << "module identified" << module;
    return module;
}

QString Python2Session::identifyVariableModule(QString command)
{
    QString variable;

    if(command.contains("import ")){
        variable = command.section(" ", 1, 1);
    }

    if((command.contains("import ")) && (command.contains(" as "))){
        variable = command.section(" ", 3, 3);
    }

    if(command.contains("from ")){
        variable = "";
    }

    kDebug() << "variable identified" << variable;
    return variable;
}

void Python2Session::expressionFinished()
{
    kDebug()<< "finished";
    Python2Expression* expression = qobject_cast<Python2Expression*>(sender());

    m_runningExpressions.removeAll(expression);
    kDebug() << "size: " << m_runningExpressions.size();
}

void Python2Session::readOutput(Python2Expression* expr, QString commandProcessing)
{
    kDebug() << "readOutput";

    getPythonCommandOutput(commandProcessing);

    if(m_error.isEmpty()){

        expr->parseOutput(m_output);

        kDebug() << "output: " << m_output;

    } else {

        expr->parseError(m_error);

        kDebug() << "error: " << m_error;
    }

    listVariables();

    changeStatus(Cantor::Session::Done);
}

void Python2Session::plotFileChanged(QString filename)
{
    kDebug() << "plotFileChanged filename:" << filename;

    if ((m_currentExpression) && (filename.contains("cantor-export-python-figure")))
    {
         kDebug() << "Calling parsePlotFile";
         m_currentExpression->parsePlotFile(filename);

         m_listPlotName.append(filename);
    }
}

void Python2Session::listVariables()
{
    QString listVariableCommand;
    listVariableCommand += "print globals()\n";

    getPythonCommandOutput(listVariableCommand);

    kDebug() << m_output;

    m_output.remove("{");
    m_output.remove("<");
    m_output.remove(">");
    m_output.remove("}");

    kDebug() << m_output;

    foreach(QString line, m_output.split(", '")){

        QStringList parts = line.simplified().split(":");

        if(!parts.first().startsWith("'__") && !parts.first().startsWith("__") && !parts.first().startsWith("CatchOutPythonBackend'") &&
           !parts.first().startsWith("errorPythonBackend'") && !parts.first().startsWith("outputPythonBackend'") &&
           !parts.first().startsWith("sys':") && !parts.last().startsWith(" class ") && !parts.last().startsWith(" function ")){

            m_variableModel->addVariable(parts.first().remove("'").simplified(), parts.last().simplified());
            Python2Keywords::instance()->addVariable(parts.first().remove("'").simplified());

        }
    }

    kDebug() << "emitting updateHighlighter";
    emit updateHighlighter();

}

QSyntaxHighlighter* Python2Session::syntaxHighlighter(QObject* parent)
{
    Python2Highlighter* highlighter = new Python2Highlighter(parent);
    QObject::connect(this, SIGNAL(updateHighlighter()), highlighter, SLOT(updateHighlight()));

    return highlighter;
}

Cantor::CompletionObject* Python2Session::completionFor(const QString& command, int index)
{
    return new Python2CompletionObject(command, index, this);
}

QAbstractItemModel* Python2Session::variableModel()
{
    return m_variableModel;
}

#include "python2session.moc"
