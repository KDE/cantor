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

#include <QDebug>
#include <KDirWatch>

#include <QFile>
#include <QTextEdit>
#include <QListIterator>
#include <QDir>
#include <QIODevice>
#include <QByteArray>
#include <QStringList>

#include <settings.h>
#include <defaultvariablemodel.h>

#include <string>

using namespace std;

Python2Session::Python2Session(Cantor::Backend* backend) : Session(backend),
m_variableModel(new Cantor::DefaultVariableModel(this))
{
    qDebug();
}

Python2Session::~Python2Session()
{
    qDebug();
}

void Python2Session::login()
{
    qDebug()<<"login";

    Py_Initialize();
    m_pModule = PyImport_AddModule("__main__");

    if(Python2Settings::integratePlots())
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

    if(!Python2Settings::self()->autorunScripts().isEmpty()){
        QString autorunScripts = Python2Settings::self()->autorunScripts().join(QLatin1String("\n"));
        getPythonCommandOutput(autorunScripts);
    }

    QString importDefaultModules = QLatin1String("import numpy\n"      \
                                                 "import scipy\n"      \
                                                 "import matplotlib");

    evaluateExpression(importDefaultModules, Cantor::Expression::DeleteOnFinish);

    listVariables();

    emit ready();
}

void Python2Session::logout()
{
    qDebug()<<"logout";

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(QLatin1String(i.next().toLocal8Bit().constData()));
    }

    changeStatus(Cantor::Session::Done);
}

void Python2Session::interrupt()
{
    qDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* Python2Session::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    qDebug() << "evaluating: " << cmd;
    Python2Expression* expr = new Python2Expression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void Python2Session::runExpression(Python2Expression* expr)
{
    qDebug() << "run expression";

    m_currentExpression = expr;

    QString command;

    command += expr->command();

    QStringList commandLine = command.split(QLatin1String("\n"));

    QString commandProcessing;

    for(int contLine = 0; contLine < commandLine.size(); contLine++){

        QString firstLineWord = commandLine.at(contLine).trimmed().replace(QLatin1String("("), QLatin1String(" "))
            .split(QLatin1String(" ")).at(0);

        if(commandLine.at(contLine).contains(QLatin1String("import "))){

            if(identifyKeywords(commandLine.at(contLine).simplified())){
                continue;
            } else {
                readOutput(expr, commandLine.at(contLine).simplified());
                return;
            }
        }

        if(firstLineWord.contains(QLatin1String("execfile"))){

            commandProcessing += commandLine.at(contLine);
            continue;
        }

        if((!Python2Keywords::instance()->keywords().contains(firstLineWord)) && (!commandLine.at(contLine).contains(QLatin1String("="))) &&
           (!commandLine.at(contLine).endsWith(QLatin1String(":"))) && (!commandLine.at(contLine).startsWith(QLatin1String(" ")))){

            commandProcessing += QLatin1String("print ") + commandLine.at(contLine) + QLatin1String("\n");

            continue;
        }

        if(commandLine.at(contLine).startsWith(QLatin1String(" "))){

            if((Python2Keywords::instance()->keywords().contains(firstLineWord)) || (commandLine.at(contLine).contains(QLatin1String("="))) ||
               (commandLine.at(contLine).endsWith(QLatin1String(":")))){

                commandProcessing += commandLine.at(contLine) + QLatin1String("\n");

                continue;
            }

            int contIdentationSpace;

            for(contIdentationSpace = 0; commandLine.at(contLine).at(contIdentationSpace).isSpace(); contIdentationSpace++);

            qDebug() << "contIdentationSpace: " << contIdentationSpace;

            QString commandIdentation;

            commandIdentation = commandLine.at(contLine);

            qDebug() << "Insert print in " << contIdentationSpace << "space";
            qDebug() << "commandIdentation before insert " << commandIdentation;

            commandIdentation.insert(contIdentationSpace, QLatin1String("print "));

            qDebug() << "commandIdentation after insert" << commandIdentation;

            commandProcessing += commandIdentation + QLatin1String("\n");

            continue;
        }

        commandProcessing += commandLine.at(contLine) + QLatin1String("\n");

    }

    readOutput(expr, commandProcessing);
}

void Python2Session::runClassOutputPython()
{
    QString classOutputPython = QLatin1String("import sys\n"                                      \
                                              "class CatchOutPythonBackend:\n"                    \
                                              "    def __init__(self):\n"                         \
                                              "        self.value = ''\n"                         \
                                              "    def write(self, txt):\n"                       \
                                              "        self.value += txt\n"                       \
                                              "outputPythonBackend = CatchOutPythonBackend()\n"   \
                                              "errorPythonBackend  = CatchOutPythonBackend()\n"   \
                                              "sys.stdout = outputPythonBackend\n"   \
                                              "sys.stderr = errorPythonBackend\n");

    PyRun_SimpleString(classOutputPython.toStdString().c_str());
}

void Python2Session::getPythonCommandOutput(QString commandProcessing)
{
    qDebug() << "Running python command" << commandProcessing.toStdString().c_str();

    runClassOutputPython();
    PyRun_SimpleString(commandProcessing.toStdString().c_str());

    PyObject *outputPython = PyObject_GetAttrString(m_pModule, "outputPythonBackend");
    PyObject *output = PyObject_GetAttrString(outputPython, "value");
    string outputString = PyString_AsString(output);

    PyObject *errorPython = PyObject_GetAttrString(m_pModule, "errorPythonBackend");
    PyObject *error = PyObject_GetAttrString(errorPython, "value");
    string errorString = PyString_AsString(error);

    m_output = QString::fromLocal8Bit(outputString.c_str());

    m_error = QString::fromLocal8Bit(errorString.c_str());
}

bool Python2Session::identifyKeywords(QString command)
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
                                            "print dir(%1)\n" \
                                            "del %1\n").arg(moduleImported);
    }

    if(!moduleVariable.isEmpty()){
        listKeywords += QLatin1String("print dir(") + moduleVariable + QLatin1String(")\n");
    }

    if(!listKeywords.isEmpty()){
        getPythonCommandOutput(listKeywords);

        keywordsString = m_output;

        keywordsString.remove(QLatin1String("'"));
        keywordsString.remove(QLatin1String(" "));
        keywordsString.remove(QLatin1String("["));
        keywordsString.remove(QLatin1String("]"));
    }

    qDebug() << "keywordsString" << keywordsString;

    QStringList keywordsList = keywordsString.split(QLatin1String(","));

    qDebug() << "keywordsList" << keywordsList;

    Python2Keywords::instance()->loadFromModule(moduleVariable, keywordsList);

    qDebug() << "Module imported" << moduleImported;

    return true;
}

QString Python2Session::identifyPythonModule(QString command)
{
    QString module;

    if(command.contains(QLatin1String("import "))){
        module = command.section(QLatin1String(" "), 1, 1);
    }

    qDebug() << "module identified" << module;
    return module;
}

QString Python2Session::identifyVariableModule(QString command)
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

void Python2Session::expressionFinished()
{
    qDebug()<< "finished";
    Python2Expression* expression = qobject_cast<Python2Expression*>(sender());

    m_runningExpressions.removeAll(expression);
    qDebug() << "size: " << m_runningExpressions.size();
}

void Python2Session::readOutput(Python2Expression* expr, QString commandProcessing)
{
    qDebug() << "readOutput";

    getPythonCommandOutput(commandProcessing);

    if(m_error.isEmpty()){

        expr->parseOutput(m_output);

        qDebug() << "output: " << m_output;

    } else {

        expr->parseError(m_error);

        qDebug() << "error: " << m_error;
    }

    listVariables();

    changeStatus(Cantor::Session::Done);
}

void Python2Session::plotFileChanged(QString filename)
{
    qDebug() << "plotFileChanged filename:" << filename;

    if ((m_currentExpression) && (filename.contains(QLatin1String("cantor-export-python-figure"))))
    {
         qDebug() << "Calling parsePlotFile";
         m_currentExpression->parsePlotFile(filename);

         m_listPlotName.append(filename);
    }
}

void Python2Session::listVariables()
{
    QString listVariableCommand;
    listVariableCommand += QLatin1String("print globals()\n");

    getPythonCommandOutput(listVariableCommand);

    qDebug() << m_output;

    m_output.remove(QLatin1String("{"));
    m_output.remove(QLatin1String("<"));
    m_output.remove(QLatin1String(">"));
    m_output.remove(QLatin1String("}"));

    qDebug() << m_output;

    foreach(QString line, m_output.split(QLatin1String(", '"))){

        QStringList parts = line.simplified().split(QLatin1String(":"));

        if(!parts.first().startsWith(QLatin1String("'__")) && !parts.first().startsWith(QLatin1String("__")) && !parts.first().startsWith(QLatin1String("CatchOutPythonBackend'")) &&
           !parts.first().startsWith(QLatin1String("errorPythonBackend'")) && !parts.first().startsWith(QLatin1String("outputPythonBackend'")) &&
           !parts.first().startsWith(QLatin1String("sys':")) && !parts.last().startsWith(QLatin1String(" class ")) && !parts.last().startsWith(QLatin1String(" function "))){

            m_variableModel->addVariable(parts.first().remove(QLatin1String("'")).simplified(), parts.last().simplified());
            Python2Keywords::instance()->addVariable(parts.first().remove(QLatin1String("'")).simplified());

        }
    }

    qDebug() << "emitting updateHighlighter";
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
