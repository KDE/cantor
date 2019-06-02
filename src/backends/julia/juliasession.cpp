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
#include <KLocalizedString>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>

#include "defaultvariablemodel.h"

#include "juliaexpression.h"
#include "settings.h"
#include "juliahighlighter.h"
#include "juliakeywords.h"
#include "juliavariablemodel.h"
#include "juliaextensions.h"
#include "juliabackend.h"
#include "juliacompletionobject.h"

using namespace Cantor;

JuliaSession::JuliaSession(Cantor::Backend *backend)
    : Session(backend)
    , m_process(nullptr)
    , m_interface(nullptr)
{
    setVariableModel(new JuliaVariableModel(this));
}

JuliaSession::~JuliaSession()
{
    if (m_process)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void JuliaSession::login()
{
    if (m_process)
        return;
    emit loginStarted();

    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);

    (*m_process)
        << QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver"));

    connect(m_process, &QProcess::errorOccurred, this, &JuliaSession::reportServerProcessError);

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

    static_cast<JuliaVariableModel*>(variableModel())->setJuliaServer(m_interface);

    // Plots integration
    if (integratePlots()) {
        runJuliaCommand(
            QLatin1String("import GR; ENV[\"GKS_WSTYPE\"] = \"nul\"")
        );
        updateVariables();
    }

    changeStatus(Session::Done);
    emit loginDone();
    qDebug() << "login to julia done";
}

void JuliaSession::logout()
{
    if(!m_process)
        return;

    if(status() == Cantor::Session::Running)
        interrupt();

    if (m_process->pid())
        m_process->terminate();
    m_process->deleteLater();
    m_process = nullptr;

    variableModel()->clearVariables();

    changeStatus(Status::Disable);
}

void JuliaSession::interrupt()
{
    if (expressionQueue().isEmpty())
        return;

    if (m_process->pid())
    {
        disconnect(m_process, &QProcess::errorOccurred, this, &JuliaSession::reportServerProcessError);
        m_process->kill();
    }

    qDebug()<<"interrupting " << expressionQueue().first()->command();
    foreach (Cantor::Expression* expression, expressionQueue())
        expression->setStatus(Cantor::Expression::Interrupted);
    expressionQueue().clear();

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression *JuliaSession::evaluateExpression(
    const QString &cmd,
    Cantor::Expression::FinishingBehavior behave,
    bool internal)
{
    JuliaExpression *expr = new JuliaExpression(this, internal);

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
    return new JuliaHighlighter(parent, this);
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
    static_cast<JuliaExpression*>(expressionQueue().first())->finalize(getOutput(), getError(), getWasException());
    finishFirstExpression(true);
}

void JuliaSession::reportServerProcessError(QProcess::ProcessError serverError)
{
    switch(serverError)
    {
        case QProcess::Crashed:
            emit error(i18n("Julia process stopped working."));
            break;

        case QProcess::FailedToStart:
            emit error(i18n("Failed to start Julia process."));
            break;

        default:
            emit error(i18n("Communication with Julia process failed for unknown reasons."));
            return;
    }
}


void JuliaSession::runFirstExpression()
{
    Cantor::Expression* expr = expressionQueue().first();
    expr->setStatus(Cantor::Expression::Computing);

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


bool JuliaSession::integratePlots()
{
    return JuliaSettings::integratePlots();
}


