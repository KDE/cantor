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

#include <defaultvariablemodel.h>
#include "pythonsession.h"
#include "pythonexpression.h"
#include "pythonvariablemodel.h"
#include "pythonhighlighter.h"
#include "pythoncompletionobject.h"
#include "pythonkeywords.h"
#include "pythonutils.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>

#include <KDirWatch>


#ifndef Q_OS_WIN
#include <signal.h>
#endif


const QChar recordSep(30);
const QChar unitSep(31);
const QChar messageEnd = 29;

PythonSession::PythonSession(Cantor::Backend* backend, int pythonVersion, const QString serverName)
    : Session(backend)
    , m_process(nullptr)
    , serverName(serverName)
    , m_pythonVersion(pythonVersion)
{
    setVariableModel(new PythonVariableModel(this));
}

PythonSession::~PythonSession()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
}

void PythonSession::login()
{
    qDebug()<<"login";
    emit loginStarted();

    if (m_process)
        m_process->deleteLater();

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::ForwardedErrorChannel);

    m_process->start(QStandardPaths::findExecutable(serverName));

    m_process->waitForStarted();
    m_process->waitForReadyRead();
    QTextStream stream(m_process->readAllStandardOutput());

    const QString& readyStatus = QString::fromLatin1("ready");
    while (m_process->state() == QProcess::Running)
    {
        const QString& rl = stream.readLine();
        if (rl == readyStatus)
            break;
    }

    connect(m_process, &QProcess::readyReadStandardOutput, this, &PythonSession::readOutput);

    sendCommand(QLatin1String("login"));
    sendCommand(QLatin1String("setFilePath"), QStringList(worksheetPath));

    const QStringList& scripts = autorunScripts();
    if(!scripts.isEmpty()){
        QString autorunScripts = scripts.join(QLatin1String("\n"));
        evaluateExpression(autorunScripts, Cantor::Expression::DeleteOnFinish, true);
        variableModel()->update();
    }

    const QString& importerFile = QLatin1String(":py/import_default_modules.py");

    evaluateExpression(fromSource(importerFile), Cantor::Expression::DeleteOnFinish, true);


    changeStatus(Session::Done);
    emit loginDone();
}

void PythonSession::logout()
{
    if (!m_process)
        return;

    sendCommand(QLatin1String("exit"));
    m_process = nullptr;

    variableModel()->clearVariables();

    qDebug()<<"logout";
    changeStatus(Status::Disable);
}

void PythonSession::interrupt()
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
        for (Cantor::Expression* expression : expressionQueue())
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

QSyntaxHighlighter* PythonSession::syntaxHighlighter(QObject* parent)
{
    return new PythonHighlighter(parent, this, m_pythonVersion);
}

Cantor::CompletionObject* PythonSession::completionFor(const QString& command, int index)
{
    return new PythonCompletionObject(command, index, this);
}

void PythonSession::runFirstExpression()
{
    if (expressionQueue().isEmpty())
        return;

    Cantor::Expression* expr = expressionQueue().first();
    const QString command = expr->internalCommand();
    qDebug() << "run first expression" << command;
    expr->setStatus(Cantor::Expression::Computing);

    if (expr->isInternal() && command.startsWith(QLatin1String("%variables ")))
    {
        const QString arg = command.section(QLatin1String(" "), 1);
        sendCommand(QLatin1String("model"), QStringList(arg));
    }
    else
        sendCommand(QLatin1String("code"), QStringList(expr->internalCommand()));
}

void PythonSession::sendCommand(const QString& command, const QStringList arguments) const
{
    qDebug() << "send command: " << command << arguments;
    const QString& message = command + recordSep + arguments.join(unitSep) + messageEnd;
    m_process->write(message.toLocal8Bit());
}

void PythonSession::readOutput()
{
    while (m_process->bytesAvailable() > 0)
        m_output.append(QString::fromLocal8Bit(m_process->readAll()));

    qDebug() << "m_output: " << m_output;

    if (!m_output.contains(messageEnd))
        return;
    const QStringList packages = m_output.split(messageEnd, QString::SkipEmptyParts);
    if (m_output.endsWith(messageEnd))
        m_output.clear();
    else
        m_output = m_output.section(messageEnd, -1);

    for (const QString& message: packages)
    {
        if (expressionQueue().isEmpty())
            break;

        const QString& output = message.section(unitSep, 0, 0);
        const QString& error = message.section(unitSep, 1, 1);
        if(error.isEmpty()){
            static_cast<PythonExpression*>(expressionQueue().first())->parseOutput(output);
        } else {
            static_cast<PythonExpression*>(expressionQueue().first())->parseError(error);
        }
        finishFirstExpression(true);
    }
}

void PythonSession::setWorksheetPath(const QString& path)
{
    worksheetPath = path;
}
