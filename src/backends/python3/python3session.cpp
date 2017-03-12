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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#include "python3session.h"
#include "settings.h"
#include "../python/pythonexpression.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include <KProcess>

Python3Session::Python3Session(Cantor::Backend* backend)
    : PythonSession(backend)
    , m_pIface(nullptr)
    , m_pProcess(nullptr)
{
}

void Python3Session::login()
{
    if (m_pProcess)
        m_pProcess->deleteLater();

    m_pProcess = new KProcess(this);
    m_pProcess->setOutputChannelMode(KProcess::SeparateChannels);

    (*m_pProcess) << QStandardPaths::findExecutable(QLatin1String("cantor_python3server"));

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

    const QString& serviceName = QString::fromLatin1("org.kde.Cantor.Python3-%1").arg(m_pProcess->pid());

    m_pIface =  new QDBusInterface(serviceName,
                                   QString::fromLatin1("/"), QString(), QDBusConnection::sessionBus());
    if (!m_pIface->isValid())
    {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    m_pIface->call(QString::fromLatin1("login"));

    PythonSession::login();
}

void Python3Session::logout()
{
    m_pProcess->terminate();
    PythonSession::logout();
}

void Python3Session::interrupt()
{
    if (m_pProcess->pid())
        m_pProcess->kill();

    PythonSession::interrupt();
}

void Python3Session::runPythonCommand(const QString& command) const
{
    m_pIface->call(QString::fromLatin1("runPythonCommand"), command);
}

void Python3Session::readExpressionOutput(const QString& commandProcessing)
{
    runClassOutputPython();
    runPythonCommand(commandProcessing);
    m_output = getOutput();
    m_error = getError();

    updateOutput();
}

QString Python3Session::getOutput() const
{
    const QDBusReply<QString>& reply = m_pIface->call(QString::fromLatin1("getOutput"));
    if (reply.isValid())
        return reply.value();

    return reply.error().message();
}

QString Python3Session::getError() const
{
    const QDBusReply<QString>& reply = m_pIface->call(QString::fromLatin1("getError"));
    if (reply.isValid())
        return reply.value();

    return reply.error().message();
}

bool Python3Session::integratePlots() const
{
    return PythonSettings::integratePlots();
}

QStringList Python3Session::autorunScripts() const
{
    return PythonSettings::autorunScripts();
}
