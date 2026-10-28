/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2026 Alexander Semke <alexander.semke@web.de>
*/

#include "rsession.h"
#include "rexpression.h"
#include "rvariablemodel.h"
#include "rserver/rprotocol.h"
#include "settings.h"
#include <defaultvariablemodel.h>

#include <QTimer>
#include <QDebug>
#include <QDataStream>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include <KProcess>
#include <KLocalizedString>

#ifndef Q_OS_WIN
#include <signal.h>
#endif

RSession::RSession(Cantor::Backend* backend) : Session(backend, nullptr, new KeywordsManager(QStringLiteral("R Script")))
{
    setVariableModel(new RVariableModel(this));
}

RSession::~RSession()
{
    if (m_process)
        m_process->terminate();
}

void RSession::login()
{
    qDebug()<<"login";
    if (m_process)
        return;
    Q_EMIT loginStarted();

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    // Forward server's stderr to our stderr for debugging
    connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
        qDebug().noquote() << "[rserver]" << m_process->readAllStandardError();
    });

    // Server writes socket name to stdout
    m_process->setReadChannel(QProcess::StandardOutput);

    // Find the rserver executable
    QString rserverPath;
#ifdef Q_OS_WIN
    rserverPath = QStandardPaths::findExecutable(QLatin1String("cantor_rserver.exe"));
    if (rserverPath.isEmpty()) {
        // Also check in same directory as the current executable (for CI/tests)
        QString localPath = QCoreApplication::applicationDirPath() + QLatin1String("/cantor_rserver.exe");
        if (QFile::exists(localPath))
            rserverPath = localPath;
    }
#elif defined(Q_OS_MACOS)
    // On macOS, also check in app bundle next to cantor.app
    rserverPath = QStandardPaths::findExecutable(QLatin1String("cantor_rserver"));
    if (rserverPath.isEmpty()) {
        QDir appDir(QCoreApplication::applicationDirPath());
        appDir.cdUp(); appDir.cdUp(); appDir.cdUp(); // MacOS -> Contents -> cantor.app -> bin
        QString bundlePath = appDir.filePath(QLatin1String("cantor_rserver.app/Contents/MacOS/cantor_rserver"));
        if (QFile::exists(bundlePath))
            rserverPath = bundlePath;
    }
#else
    rserverPath = QStandardPaths::findExecutable(QLatin1String("cantor_rserver"));
#endif

    if (rserverPath.isEmpty())
    {
        changeStatus(Session::Disable);
        Q_EMIT error(i18n("Failed to find cantor_rserver executable."));
        Q_EMIT loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

    m_process->start(rserverPath);

    if (!m_process->waitForStarted())
    {
        changeStatus(Session::Disable);
        Q_EMIT error(i18n("Failed to start R, please check R installation."));
        Q_EMIT loginDone();
        delete m_process;
        m_process = nullptr;
        return;
    }

    // Read socket name from server stdout
    if (!m_process->waitForReadyRead(5000))
    {
        changeStatus(Session::Disable);
        Q_EMIT error(i18n("R server did not respond."));
        Q_EMIT loginDone();
        m_process->kill();
        delete m_process;
        m_process = nullptr;
        return;
    }

    QString socketName = QString::fromUtf8(m_process->readLine()).trimmed();
    qDebug() << "Connecting to socket:" << socketName;

    // Connect to server
    m_socket = new QLocalSocket(this);
    connect(m_socket, &QLocalSocket::readyRead, this, &RSession::readFromServer);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &RSession::socketError);

    m_socket->connectToServer(socketName);
    if (!m_socket->waitForConnected(5000))
    {
        changeStatus(Session::Disable);
        Q_EMIT error(i18n("Failed to connect to R server: %1", m_socket->errorString()));
        Q_EMIT loginDone();
        m_process->kill();
        delete m_socket;
        m_socket = nullptr;
        delete m_process;
        m_process = nullptr;
        return;
    }

    // Set working directory
    const auto& path = worksheetPath();
    if (!path.isEmpty())
    {
        const auto& dir = QFileInfo(path).absoluteDir().absolutePath();
        // Send setwd command (will be implemented in runFirstExpression)
        auto* expr = evaluateExpression(QLatin1String("setwd(\"") + dir + QLatin1String("\")"),
                                       Cantor::Expression::DeleteOnFinish, true);
        Q_UNUSED(expr);
    }

    changeStatus(Session::Done);
    Q_EMIT loginDone();
    qDebug()<<"login done";
}

void RSession::logout()
{
    qDebug()<<"logout";
    if (!m_process)
        return;

    if(!expressionQueue().isEmpty())
        interrupt();

    if (m_socket)
    {
        m_socket->disconnectFromServer();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    m_process->kill();
    // Wait for process to terminate before deleting, otherwise QProcess warns
    // "Destroyed while process is still running" and may crash
    m_process->waitForFinished(1000);
    m_process->deleteLater();
    m_process = nullptr;

    Session::logout();
    qDebug()<<"logout done";
}

void RSession::interrupt()
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
            ; //TODO: send interrupt message to server
#endif
        }
        for (auto* expression : expressionQueue())
            expression->setStatus(Cantor::Expression::Interrupted);
        expressionQueue().clear();

        qDebug()<<"done interrupting";
    }

    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* RSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave, bool internal)
{
    qDebug()<<"evaluating: "<<cmd;
    auto* expr = new RExpression(this, internal);
    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);

    expr->evaluate();

    return expr;
}

void RSession::runFirstExpression()
{
    qDebug() << "RSession::runFirstExpression called, queue size:" << expressionQueue().size();

    if (expressionQueue().isEmpty())
        return;

    auto* expr = expressionQueue().first();
    qDebug()<<"running expression: "<<expr->command();

    expr->setStatus(Cantor::Expression::Computing);
    changeStatus(Cantor::Session::Running);

    // Send RunCommand message with current plot settings
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    RProtocol::writeString(stream, expr->internalCommand());
    stream << static_cast<quint8>(expr->isInternal() ? 1 : 0);

    // Include plot settings so rserver uses current values
    quint8 plotFormat = static_cast<quint8>(RServerSettings::self()->inlinePlotFormat());
    double plotWidth = RServerSettings::self()->plotWidth();
    double plotHeight = RServerSettings::self()->plotHeight();
    stream << plotFormat << plotWidth << plotHeight;

    qDebug() << "RSession: sending command:" << expr->internalCommand()
             << "plotFormat:" << plotFormat << "size:" << plotWidth << "x" << plotHeight;
    sendMessage(static_cast<quint8>(RProtocol::ClientMessage::RunCommand), data);
}

void RSession::sendInputToServer(const QString& input)
{
    QString s = input;
    if(!input.endsWith(QLatin1Char('\n')))
        s += QLatin1Char('\n');

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    RProtocol::writeString(stream, s);

    sendMessage(static_cast<quint8>(RProtocol::ClientMessage::AnswerRequest), data);
}

void RSession::sendMessage(quint8 msgType, const QByteArray& data)
{
    if (!m_socket || m_socket->state() != QLocalSocket::ConnectedState)
    {
        qWarning() << "RSession: socket not connected, cannot send message";
        return;
    }

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    // Message format: length(quint32) | type(quint8) | data
    stream << static_cast<quint32>(data.size() + 1); // +1 for type byte
    stream << msgType;
    stream.writeRawData(data.constData(), data.size());

    qDebug() << "RSession: sending message type:" << msgType << "size:" << message.size();

    qint64 written = m_socket->write(message);
    m_socket->flush();
    qDebug() << "RSession: wrote" << written << "bytes";
}

void RSession::readFromServer()
{
    m_receiveBuffer.append(m_socket->readAll());

    // Process complete messages
    while (m_receiveBuffer.size() >= 5) // At least length + type
    {
        QDataStream stream(m_receiveBuffer);
        stream.setVersion(QDataStream::Qt_6_0);

        quint32 messageLen;
        stream >> messageLen;

        if (m_receiveBuffer.size() < static_cast<int>(messageLen + 4)) // +4 for length field
            break; // Wait for complete message

        quint8 msgType;
        stream >> msgType;

        // Process message based on type
        switch (static_cast<RProtocol::ServerMessage>(msgType))
        {
            case RProtocol::ServerMessage::StatusChanged:
                handleStatusChanged(stream);
                break;
            case RProtocol::ServerMessage::ExpressionFinished:
                handleExpressionFinished(stream);
                break;
            case RProtocol::ServerMessage::InputRequested:
                handleInputRequested(stream);
                break;
            case RProtocol::ServerMessage::PartialOutput:
                handlePartialOutput(stream);
                break;
            default:
                qWarning() << "Unknown message type:" << msgType;
                break;
        }

        // Remove processed message
        m_receiveBuffer.remove(0, messageLen + 4);
    }
}

void RSession::handleStatusChanged(QDataStream& stream)
{
    quint8 status;
    stream >> status;

    qDebug() << "Status changed:" << status;

    if (status == static_cast<quint8>(RProtocol::Status::Idle))
    {
        if (expressionQueue().isEmpty())
            changeStatus(Cantor::Session::Done);
    }
    else
    {
        changeStatus(Cantor::Session::Running);
    }
}

void RSession::handleExpressionFinished(QDataStream& stream)
{
    if (expressionQueue().isEmpty())
        return;

    quint8 returnCode;
    stream >> returnCode;

    QString text = RProtocol::readString(stream);
    QStringList files = RProtocol::readStringList(stream);

    auto* expr = expressionQueue().first();
    const bool isInternal = expr->isInternal();
    qDebug() << "handleExpressionFinished: returnCode=" << returnCode
             << "text=" << (isInternal ? text.left(80) + QLatin1String("...") : text)
             << "files=" << files;
    if (expr->status() == Cantor::Expression::Interrupted)
        return;

    static_cast<RExpression*>(expr)->showFilesAsResult(files);

    if (returnCode == static_cast<quint8>(RProtocol::ReturnCode::Success))
        expr->parseOutput(text);
    else
        expr->parseError(text);

    qDebug()<<"done running "<<expr<<" "<<expr->command();
    finishFirstExpression();
}

void RSession::handleInputRequested(QDataStream& stream)
{
    if (expressionQueue().isEmpty())
        return;

    QString info = RProtocol::readString(stream);
    Q_EMIT expressionQueue().first()->needsAdditionalInformation(info);
}

void RSession::socketError()
{
    qWarning() << "Socket error:" << m_socket->errorString();
    if (m_socket->state() == QLocalSocket::UnconnectedState)
    {
        changeStatus(Cantor::Session::Done);
        Q_EMIT error(i18n("Lost connection to R server"));
    }
}

void RSession::handlePartialOutput(QDataStream& stream)
{
    if (expressionQueue().isEmpty())
        return;

    QString text = RProtocol::readString(stream);
    static_cast<RExpression*>(expressionQueue().first())->parsePartialOutput(text);
}
