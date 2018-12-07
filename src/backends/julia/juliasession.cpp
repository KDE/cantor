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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliasession.h"

#include <KProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>

#include "defaultvariablemodel.h"

#include "juliaexpression.h"
#include "settings.h"
#include "juliahighlighter.h"
#include "juliakeywords.h"
#include "juliaextensions.h"
#include "juliabackend.h"
#include "juliacompletionobject.h"
#include <julia/julia_version.h>

const QRegularExpression JuliaSession::typeVariableInfo = QRegularExpression(QLatin1String("\\w+\\["));

JuliaSession::JuliaSession(Cantor::Backend *backend)
    : Session(backend)
    , m_process(nullptr)
    , m_interface(nullptr)
    , m_currentExpression(nullptr)
    , m_variableModel(new Cantor::DefaultVariableModel(this))
{
}

void JuliaSession::login()
{
    emit loginStarted();

    if (m_process) {
        m_process->deleteLater();
    }

    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);

    (*m_process)
        << QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver"));

    m_process->start();

    m_process->waitForStarted();
    m_process->waitForReadyRead();
    QTextStream stream(m_process->readAllStandardOutput());

    QString readyStatus = QLatin1String("ready");
    while (m_process->state() == QProcess::Running) {
        const QString &rl = stream.readLine();
        if (rl == readyStatus) {
            break;
        }
    }

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Can't connect to the D-Bus session bus.\n"
                      "To start it, run: eval `dbus-launch --auto-syntax`";
        return;
    }

    const QString &serviceName =
        QString::fromLatin1("org.kde.Cantor.Julia-%1").arg(m_process->pid());

    m_interface = new QDBusInterface(
        serviceName,
        QString::fromLatin1("/"),
        QString(),
        QDBusConnection::sessionBus()
    );

    if (!m_interface->isValid()) {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    m_interface->call(
        QString::fromLatin1("login"),
        JuliaSettings::self()->replPath().path()
    );

    listVariables();

    // Plots integration
    if (integratePlots()) {
        runJuliaCommand(
            QLatin1String("import GR; ENV[\"GKS_WSTYPE\"] = \"nul\"")
        );
    }

    changeStatus(Session::Done);
    emit loginDone();
    qDebug() << "login to julia " << JULIA_VERSION_STRING << "done";
}

void JuliaSession::logout()
{
    m_process->terminate();

    JuliaKeywords::instance()->clearVariables();
    JuliaKeywords::instance()->clearFunctions();
    m_variableModel->clearVariables();
    emit updateHighlighter();

    changeStatus(Status::Disable);
}

void JuliaSession::interrupt()
{
    if (m_process->pid()) {
        m_process->kill();
    }

    for (Cantor::Expression *e : m_runningExpressions) {
        e->interrupt();
    }

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression *JuliaSession::evaluateExpression(
    const QString &cmd,
    Cantor::Expression::FinishingBehavior behave,
    bool internal)
{
    JuliaExpression *expr = new JuliaExpression(this, internal);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

Cantor::CompletionObject *JuliaSession::completionFor(
    const QString &command,
    int index)
{
    return new JuliaCompletionObject(command, index, this);
}

QSyntaxHighlighter *JuliaSession::syntaxHighlighter(QObject *parent)
{
    JuliaHighlighter *highlighter = new JuliaHighlighter(parent);
    QObject::connect(
        this, SIGNAL(updateHighlighter()), highlighter, SLOT(updateHighlight())
    );
    return highlighter;
}

void JuliaSession::runJuliaCommand(const QString &command) const
{
    m_interface->call(QLatin1String("runJuliaCommand"), command);
}

void JuliaSession::runJuliaCommandAsync(const QString &command)
{
    m_interface->callWithCallback(
        QLatin1String("runJuliaCommand"),
        {command},
        this,
        SLOT(onResultReady())
    );
}

void JuliaSession::onResultReady()
{
    m_currentExpression->finalize();
    m_runningExpressions.removeAll(m_currentExpression);

    listVariables();

    changeStatus(Cantor::Session::Done);
}

void JuliaSession::runExpression(JuliaExpression *expr)
{
    m_runningExpressions.append(expr);
    m_currentExpression = expr;
    runJuliaCommandAsync(expr->internalCommand());
}

QString JuliaSession::getStringFromServer(const QString &method)
{
    const QDBusReply<QString> &reply = m_interface->call(method);
    return (reply.isValid() ? reply.value() : reply.error().message());
}

QString JuliaSession::getOutput()
{
    return getStringFromServer(QLatin1String("getOutput"));
}

QString JuliaSession::getError()
{
    return getStringFromServer(QLatin1String("getError"));
}

bool JuliaSession::getWasException()
{
    const QDBusReply<bool> &reply =
        m_interface->call(QLatin1String("getWasException"));
    return reply.isValid() && reply.value();
}

void JuliaSession::listVariables()
{
    JuliaKeywords::instance()->clearVariables();
    JuliaKeywords::instance()->clearFunctions();

    m_interface->call(QLatin1String("parseModules"));

    const QStringList& variables =
        static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("variablesList"))).value();
    const QStringList& values =
        static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("variableValuesList"))).value();
    for (int i = 0; i < variables.size(); i++)
    {
        if (i >= values.size())
        {
            qWarning() << "Don't have value for variable from julia server response, something wrong!";
            continue;
        }

        const QString& name = variables[i];
        QString value = values[i];
        if (value != JuliaVariableManagementExtension::REMOVED_VARIABLE_MARKER)
        {
            // Register variable
            // We use replace here, because julia return data type for some variables, and we need
            // remove it to make variable view more consistent with the other backends
            // More info: https://bugs.kde.org/show_bug.cgi?id=377771
            m_variableModel->addVariable(name, value.replace(typeVariableInfo,QLatin1String("[")));
            JuliaKeywords::instance()->addVariable(name);
        }
        else
            m_variableModel->removeVariable(name);
    }

    const QStringList& functions =
        static_cast<QDBusReply<QStringList>>(m_interface->call(QLatin1String("functionsList"))).value();
    foreach (const QString& name, functions)
    {
        JuliaKeywords::instance()->addFunction(name);
    }

    emit updateHighlighter();
}

QAbstractItemModel *JuliaSession::variableModel()
{
    return m_variableModel;
}


bool JuliaSession::integratePlots()
{
    return JuliaSettings::integratePlots();
}


