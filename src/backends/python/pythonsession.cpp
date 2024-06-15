/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Filipe Saraiva <filipe@kde.org>
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#include <defaultvariablemodel.h>
#include <backend.h>
#include "pythonsession.h"
#include "pythonexpression.h"
#include "pythonvariablemodel.h"
#include "pythonhighlighter.h"
#include "pythoncompletionobject.h"
#include "pythonkeywords.h"
#include "pythonutils.h"
#include "settings.h"

#include <random>

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>

#include <KLocalizedString>
#include <KMessageBox>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

const QChar recordSep(30);
const QChar unitSep(31);
const QChar messageEnd = 29;

PythonSession::PythonSession(Cantor::Backend* backend) : Session(backend)
{
    setVariableModel(new PythonVariableModel(this));
}

PythonSession::~PythonSession()
{
    if (m_process) {
        disconnect(m_process, &QProcess::errorOccurred, this, &PythonSession::reportServerProcessError);
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
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

#ifdef Q_OS_WIN
    const QString& serverExecutablePath = QStandardPaths::findExecutable(QLatin1String("cantor_pythonserver.exe"));
	// On Windows QProcess can't handle paths with spaces, so add escaping
	m_process->start(QLatin1String("\"") + serverExecutablePath + QLatin1String("\""));
#else
    const QString& serverExecutablePath = QStandardPaths::findExecutable(QLatin1String("cantor_pythonserver"));
    m_process->start(serverExecutablePath);
#endif

    if (!m_process->waitForStarted())
    {
        changeStatus(Session::Disable);
        emit error(i18n("Failed to start Python, please check Python installation."));
        emit loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

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
    connect(m_process, &QProcess::errorOccurred, this, &PythonSession::reportServerProcessError);

    sendCommand(QLatin1String("login"));
    const auto& path = worksheetPath();
    if (!path.isEmpty())
    {
        const auto& dir = QFileInfo(path).absoluteDir().absolutePath();
        sendCommand(QLatin1String("setFilePath"), QStringList() << path << dir);
    }

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> rand_dist(0, 999999999);
    m_plotFilePrefixPath =
        QDir::tempPath()
        + QLatin1String("/cantor_python_")
        + QString::number(m_process->processId())
        + QLatin1String("_")
        + QString::number(rand_dist(mt))
        + QLatin1String("_");

    m_plotFileCounter = 0;
    evaluateExpression(QLatin1String("__cantor_plot_global_counter__ = 0"), Cantor::Expression::DeleteOnFinish, true);

    const QStringList& scripts = PythonSettings::autorunScripts();
    if(!scripts.isEmpty()){
        QString autorunScripts = scripts.join(QLatin1String("\n"));
        evaluateExpression(autorunScripts, Cantor::Expression::DeleteOnFinish, true);
        variableModel()->update();
    }

    changeStatus(Session::Done);
    emit loginDone();
}

void PythonSession::logout()
{
    if (!m_process)
        return;

    if (m_process->exitStatus() != QProcess::CrashExit && m_process->error() != QProcess::WriteError)
        sendCommand(QLatin1String("exit"));

    if(m_process->state() == QProcess::Running && !m_process->waitForFinished(1000))
    {
        disconnect(m_process, &QProcess::errorOccurred, this, &PythonSession::reportServerProcessError);
        m_process->kill();
        qDebug()<<"cantor_python server still running, process kill enforced";
    }
    m_process->deleteLater();
    m_process = nullptr;

    if (!m_plotFilePrefixPath.isEmpty())
    {
        for (int i = 0; i < m_plotFileCounter; i++)
            QFile::remove(m_plotFilePrefixPath + QString::number(i) + QLatin1String(".png"));
        m_plotFilePrefixPath.clear();
        m_plotFileCounter = 0;
    }

    qDebug()<<"logout";
    Session::logout();
}

void PythonSession::interrupt()
{
    if(!expressionQueue().isEmpty())
    {
        qDebug()<<"interrupting " << expressionQueue().first()->command();
        if(m_process && m_process->state() != QProcess::NotRunning)
        {
#ifndef Q_OS_WIN
            const int pid = m_process->processId();
            kill(pid, SIGINT);
#else
            ; //TODO: interrupt the process on windows
#endif
        }
        for (auto* expression : expressionQueue())
            expression->setStatus(Cantor::Expression::Interrupted);
        expressionQueue().clear();

        m_output.clear();

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* PythonSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug() << "evaluating: " << cmd;
    auto* expr = new PythonExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

QSyntaxHighlighter* PythonSession::syntaxHighlighter(QObject* parent)
{
    return new PythonHighlighter(parent, this);
}

Cantor::CompletionObject* PythonSession::completionFor(const QString& command, int index)
{
    return new PythonCompletionObject(command, index, this);
}

void PythonSession::runFirstExpression()
{
    if (expressionQueue().isEmpty())
        return;

    auto* expr = expressionQueue().first();
    const QString& command = expr->internalCommand();
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
    {
        const QByteArray& bytes = m_process->readAll();
        m_output.append(QString::fromUtf8(bytes));
    }

    qDebug() << "m_output: " << m_output;

    if (!m_output.contains(messageEnd))
        return;

    const QStringList packages = m_output.split(messageEnd, Qt::SkipEmptyParts);
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
        bool isError = message.section(unitSep, 2, 2).toInt();
        auto* expr = expressionQueue().first();
        if (isError)
        {
            if(error.isEmpty()){
                expr->parseOutput(output);
            } else {
                expr->parseError(error);
            }
        }
        else
        {
            static_cast<PythonExpression*>(expr)->parseWarning(error);
            expr->parseOutput(output);
        }
        finishFirstExpression(true);
    }
}

void PythonSession::reportServerProcessError(QProcess::ProcessError serverError)
{
    switch(serverError)
    {
        case QProcess::Crashed:
            emit error(i18n("Cantor Python server stopped working."));
            break;

        case QProcess::FailedToStart:
            emit error(i18n("Failed to start Cantor python server."));
            break;

        default:
            emit error(i18n("Communication with Cantor python server failed for unknown reasons."));
            break;
    }
    reportSessionCrash();
}

int& PythonSession::plotFileCounter()
{
    return m_plotFileCounter;
}

QString PythonSession::plotFilePrefixPath()
{
    return m_plotFilePrefixPath;
}

void PythonSession::updateGraphicPackagesFromSettings()
{
    updateEnabledGraphicPackages(backend()->availableGraphicPackages(), m_plotFilePrefixPath);
}

QString PythonSession::graphicPackageErrorMessage(QString packageId) const
{
    if (packageId == QLatin1String("matplotlib"))
    {
        return i18n(
            "For using integrated graphics with Matplotlib package, you need to install \"matplotlib\" python package first."
        );
    }
    else if (packageId == QLatin1String("plotly"))
    {
        return i18n(
            "For using integrated graphic with Plot.ly, you need to install \"plotly\" python package and special Plot.ly-compatible "
            "\"orca\" executable. See \"Static Image Export\" article in Plot.ly documentation for details."
        );
    }
    return QString();
}

