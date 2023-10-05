/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#include "juliasession.h"

#include <random>

#include <KProcess>
#include <KMessageBox>
#include <KLocalizedString>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>
#include <QDir>

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

JuliaSession::JuliaSession(Cantor::Backend* backend) : Session(backend)
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
    Q_EMIT loginStarted();

    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);

#ifdef Q_OS_WIN
    (*m_process)
        << QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver.exe"));
#else
    (*m_process)
        << QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver"));
#endif

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
        QString::fromLatin1("org.kde.Cantor.Julia-%1").arg(m_process->processId());

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

    const QDBusReply<int> &reply = m_interface->call(QLatin1String("login"));
    if (reply.isValid())
    {
        int errorCode = reply.value();
        if (errorCode != 0)
        {
            if (errorCode == 1)
            {
                const QString& juliaSysimgMissingFilepath = getError();
                KMessageBox::error(nullptr, i18n("Julia session can't login due internal julia problem with missing internal file - \"%1\"", juliaSysimgMissingFilepath), i18n("Error - Cantor"));
                return;
            }
            else // All error codes should be supported
                assert(false);
        }
    }
    else
    {
        KMessageBox::error(nullptr, i18n("Julia session can't login due unknown internal problem"), i18n("Error - Cantor"));
        return;
    }

    static_cast<JuliaVariableModel*>(variableModel())->setJuliaServer(m_interface);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> rand_dist(0, 999999999);
    m_plotFilePrefixPath =
        QDir::tempPath()
        + QLatin1String("/cantor_octave_")
        + QString::number(m_process->processId())
        + QLatin1String("_")
        + QString::number(rand_dist(mt))
        + QLatin1String("_");

    updateVariables();

    changeStatus(Session::Done);
    Q_EMIT loginDone();
    qDebug() << "login to julia done";
}

void JuliaSession::logout()
{
    if(!m_process)
        return;

    if(status() == Cantor::Session::Running)
    {
        if(m_process && m_process->state() == QProcess::Running)
        {
            disconnect(m_process, &QProcess::errorOccurred, this, &JuliaSession::reportServerProcessError);
            m_process->kill();
        }
        m_process->deleteLater();
        m_process = nullptr;
        interrupt();
    }

    if (!m_plotFilePrefixPath.isEmpty())
    {
        int i = 0;
        const QString& extension = JuliaExpression::plotExtensions[JuliaSettings::inlinePlotFormat()];
        QString filename = m_plotFilePrefixPath + QString::number(i) + QLatin1String(".") + extension;
        while (QFile::exists(filename))
        {
            QFile::remove(filename);
            i++;
            filename = m_plotFilePrefixPath + QString::number(i) + QLatin1String(".") + extension;
        }
    }
    m_isIntegratedPlotsEnabled = false;
    m_isIntegratedPlotsSettingsEnabled = false;

    Session::logout();
}

void JuliaSession::interrupt()
{
    if (expressionQueue().isEmpty())
        return;

    if (m_process && m_process->processId())
    {
        disconnect(m_process, &QProcess::errorOccurred, this, &JuliaSession::reportServerProcessError);
        m_process->kill();
    }

    qDebug()<<"interrupting " << expressionQueue().first()->command();
    for(auto* expression : expressionQueue())
        expression->setStatus(Cantor::Expression::Interrupted);

    expressionQueue().clear();

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression *JuliaSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    auto* expr = new JuliaExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

Cantor::CompletionObject* JuliaSession::completionFor(const QString &command, int index)
{
    return new JuliaCompletionObject(command, index, this);
}

QSyntaxHighlighter* JuliaSession::syntaxHighlighter(QObject *parent)
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
            Q_EMIT error(i18n("Julia process stopped working."));
            break;
        case QProcess::FailedToStart:
            Q_EMIT error(i18n("Failed to start Julia process."));
            break;
        default:
            Q_EMIT error(i18n("Communication with Julia process failed for unknown reasons."));
            break;
    }
    qDebug() << "reportSessionCrash" << serverError;
    reportSessionCrash();
}

void JuliaSession::runFirstExpression()
{
    auto* expr = expressionQueue().first();
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

QString JuliaSession::plotFilePrefixPath() const
{
    return m_plotFilePrefixPath;
}

void JuliaSession::updateGraphicPackagesFromSettings()
{
    if (m_isIntegratedPlotsSettingsEnabled == JuliaSettings::integratePlots())
        return;

    if (m_isIntegratedPlotsEnabled && JuliaSettings::integratePlots() == false)
    {
        updateEnabledGraphicPackages(QList<Cantor::GraphicPackage>());
        m_isIntegratedPlotsEnabled = false;
        m_isIntegratedPlotsSettingsEnabled = true;
        return;
    }
    else if (!m_isIntegratedPlotsEnabled && JuliaSettings::integratePlots() == true)
    {
        m_isIntegratedPlotsEnabled = true;
        m_isIntegratedPlotsSettingsEnabled = true;

        updateEnabledGraphicPackages(backend()->availableGraphicPackages());
    }
}

QString JuliaSession::graphicPackageErrorMessage(QString packageId) const
{
    QString text;

    if (packageId == QLatin1String("gr")) {
        return i18n(
            "For Julia only GR (https://gr-framework.org/), a framework for visualization applications, is supported at the moment. "
            "This package has to be installed first, if not done yet. "
            "For this, run Pkg.install(\"GR\") in Cantor or in Julia REPL. "
            "Note, this operation can take some time and it's better to perform it in Julia REPL that is able to show the current progress of the package installation."
        );
    }
    return text;
}


