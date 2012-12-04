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

#include "pythonsession.h"
#include "pythonexpression.h"
// #include "pythonhighlighter.h"
// #include "pythoncompletionobject.h"

#include <kdebug.h>
#include <KDirWatch>

#include <QtCore/QFile>
#include <QTextEdit>
#include <QListIterator>
#include <QDir>
#include <QIODevice>
#include <QByteArray>

#include <settings.h>
#include <qdir.h>

#include <Python.h>
#include <string>

using namespace std;

PythonSession::PythonSession( Cantor::Backend* backend) : Session(backend)
{
    kDebug();
}

PythonSession::~PythonSession()
{
    kDebug();
}

void PythonSession::login()
{
    kDebug()<<"login";

    Py_Initialize();
    emit ready();
}

void PythonSession::logout()
{
    kDebug()<<"logout";

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(i.next().toLocal8Bit().constData());
    }

    changeStatus(Cantor::Session::Done);
}

void PythonSession::interrupt()
{
    kDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* PythonSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug() << "evaluating: " << cmd;
    PythonExpression* expr = new PythonExpression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void PythonSession::runExpression(PythonExpression* expr)
{
    kDebug() << "run expression";

    QString command;

    command += expr->command();

    QString classOutputPython =
"import sys\n\
class CatchOut:\n\
    def __init__(self):\n\
        self.value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
output = CatchOut()\n\
sys.stdout = output\n\
sys.stderr = output\n\
";

    PyObject *pModule = PyImport_AddModule("__main__");
    PyRun_SimpleString(classOutputPython.toStdString().c_str());

    PyRun_SimpleString(command.toStdString().c_str());
    PyObject *outputPython = PyObject_GetAttrString(pModule,"output");

    PyObject *output = PyObject_GetAttrString(outputPython,"value");

    string outputString = PyString_AsString(output);

    m_output = QString(outputString.c_str());
    expr->parseOutput(m_output);
    expr->evalFinished();
    changeStatus(Cantor::Session::Done);
}

void PythonSession::expressionFinished()
{
    kDebug()<<"finished";
    PythonExpression* expression = qobject_cast<PythonExpression*>(sender());

    m_runningExpressions.removeAll(expression);
    kDebug() << "size: " << m_runningExpressions.size();
}

// void PythonSession::readOutput()
// {
//     kDebug() << "readOutput";
//
//     kDebug() << "output.isNull? " << m_output.isNull();
//     kDebug() << "output: " << m_output;
//
//     if(status() != Running || m_output.isNull()){
//         return;
//     }
// }

// void PythonSession::plotFileChanged(QString filename)
// {
//     kDebug() << "plotFileChanged filename:" << filename;
//
//     if ((m_currentExpression) && (filename.contains("cantor-export-scilab-figure")))
//     {
//          kDebug() << "Calling parsePlotFile";
//          m_currentExpression->parsePlotFile(filename);
//
//          m_listPlotName.append(filename);
//     }
// }

void PythonSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
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

// QSyntaxHighlighter* PythonSession::syntaxHighlighter(QTextEdit* parent)
// {
//     return new PythonHighlighter(parent);
// }
//
// Cantor::CompletionObject* PythonSession::completionFor(const QString& command, int index)
// {
//     return new PythonCompletionObject(command, index, this);
// }

#include "pythonsession.moc"
