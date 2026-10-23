/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
    SPDX-FileCopyrightText: 2023 by Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2025 The Cantor Team
*/

#include "rserver.h"
#include "rprotocol.h"
#include "rcallbacks.h"
#include "settings.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDataStream>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#include <KIO/DeleteJob>
#include <KLocalizedString>

#ifdef Q_OS_WIN
#include <process.h>
#else
#include <unistd.h>
#endif

//R includes
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <R_ext/Parse.h>

RServer::RServer() :
    m_isInitialized(false),
    m_server(nullptr),
    m_client(nullptr)
{
    m_tmpDir = QDir::tempPath() + QString::fromLatin1("/cantor_rserver-%1").arg(
#ifdef Q_OS_WIN
        _getpid()
#else
        getpid()
#endif
    );
    QDir dir;
    dir.mkdir(m_tmpDir);
    qDebug()<<"RServer: storing plots at"<<m_tmpDir;

    // Create local server
    m_server = new QLocalServer(this);

    // Generate unique socket name
    QString socketName = QString::fromLatin1("cantor_rserver_%1").arg(
#ifdef Q_OS_WIN
        _getpid()
#else
        getpid()
#endif
    );

    if (!m_server->listen(socketName))
    {
        qCritical() << "Failed to create server:" << m_server->errorString();
        return;
    }

    // Print socket name to stdout for client to connect
    printf("%s\n", m_server->fullServerName().toUtf8().constData());
    fflush(stdout);

    connect(m_server, &QLocalServer::newConnection, this, &RServer::newConnection);

    initR();
    m_isInitialized = true;
}

RServer::~RServer()
{
    //delete the directory with old plots
    KIO::del(QUrl::fromLocalFile(m_tmpDir));

    if (m_server)
    {
        m_server->close();
    }
}

void RServer::newConnection()
{
    if (m_client)
    {
        qWarning() << "Already have a client, rejecting new connection";
        QLocalSocket* rejected = m_server->nextPendingConnection();
        rejected->close();
        rejected->deleteLater();
        return;
    }

    m_client = m_server->nextPendingConnection();
    connect(m_client, &QLocalSocket::readyRead, this, &RServer::readFromClient);
    connect(m_client, &QLocalSocket::disconnected, this, &RServer::clientDisconnected);

    qDebug() << "Client connected";
}

void RServer::readFromClient()
{
    if (!m_client)
        return;

    QByteArray newData = m_client->readAll();
    qDebug() << "RServer: received" << newData.size() << "bytes from client";
    m_receiveBuffer.append(newData);

    // Process complete messages
    while (m_receiveBuffer.size() >= 5) // At least length + type
    {
        QDataStream stream(m_receiveBuffer);
        stream.setVersion(QDataStream::Qt_6_0);

        quint32 messageLen;
        stream >> messageLen;

        qDebug() << "RServer: message length:" << messageLen << "buffer size:" << m_receiveBuffer.size();

        if (m_receiveBuffer.size() < static_cast<int>(messageLen + 4)) // +4 for length field
            break; // Wait for complete message

        quint8 msgType;
        stream >> msgType;

        qDebug() << "RServer: message type:" << msgType;

        // Process message based on type
        switch (static_cast<RProtocol::ClientMessage>(msgType))
        {
            case RProtocol::ClientMessage::RunCommand:
                handleRunCommand(stream);
                break;
            case RProtocol::ClientMessage::AnswerRequest:
                handleAnswerRequest(stream);
                break;
            case RProtocol::ClientMessage::Interrupt:
                // TODO: implement interrupt
                break;
            default:
                qWarning() << "Unknown message type:" << msgType;
                break;
        }

        // Remove processed message
        m_receiveBuffer.remove(0, messageLen + 4);
    }
}

void RServer::clientDisconnected()
{
    qDebug() << "Client disconnected";
    m_client->deleteLater();
    m_client = nullptr;
    m_receiveBuffer.clear();
}

void RServer::handleRunCommand(QDataStream& stream)
{
    QString cmd = RProtocol::readString(stream);
    quint8 internalFlag;
    stream >> internalFlag;
    bool internal = (internalFlag != 0);

    m_expressionFiles.clear();
    qDebug()<<"RServer: running" << (internal ? "internal " : "") << "command" << cmd;

    sendStatusChanged(1); // Busy

    Expression expr;
    expr.cmd = cmd;
    expr.hasOtherResults = false;
    expr.std_buffer.clear();
    expr.err_buffer.clear();

    setCurrentExpression(&expr);

    int returnCode = SuccessCode;
    QString returnText;

    //Code to evaluate an R function (taken from RInside library)
    ParseStatus status;
    SEXP cmdSexp, cmdexpr = R_NilValue;
    SEXP result = nullptr;
    int i, errorOccurred;
    QByteArray memBuf;

    memBuf.append(cmd.toUtf8());

    PROTECT(cmdSexp = allocVector(STRSXP, 1));
    SET_STRING_ELT(cmdSexp, 0, mkChar((char*)memBuf.data()));

    cmdexpr = PROTECT(R_ParseVector(cmdSexp, -1, &status, R_NilValue));

    switch (status)
    {
        case PARSE_OK:
            qDebug()<<"RServer: PARSING" << cmd << "went OK";
            for (i = 0; i < length(cmdexpr); ++i) {
                result = R_tryEval(VECTOR_ELT(cmdexpr, i), nullptr, &errorOccurred);
                if (errorOccurred)
                {
                    qDebug()<<"RServer: Error occurred.";
                    break;
                }
            }
            memBuf.clear();
            break;
        case PARSE_INCOMPLETE:
            qDebug()<<"RServer: parse incomplete..";
            break;
        case PARSE_NULL:
            qDebug()<<"RServer: ParseStatus is null:" << status;
            break;
        case PARSE_ERROR:
            qDebug()<<"RServer: Parse Error:" << cmd;
            break;
        case PARSE_EOF:
            qDebug()<<"RServer: ParseStatus is eof:" << status;
            break;
        default:
            qDebug()<<"RServer: Parse status is not documented:" << status;
            break;
    }
    UNPROTECT(2);

    if (status == PARSE_OK)
    {
        qDebug()<<"RServer: done running";
        qDebug()<<"RServer: result:" << result << "std:" << expr.std_buffer << "err:" << expr.err_buffer;

        // If the command didn't print anything on its own, print the result
        if (result && expr.std_buffer.isEmpty() && expr.err_buffer.isEmpty())
        {
            qDebug()<<"RServer: printing result...";
            SEXP count = PROTECT(R_tryEval(lang2(install("length"), result), nullptr, &errorOccurred));
            if (*INTEGER(count) == 0)
                qDebug()<<"RServer: no result, so show nothing";
            else
                Rf_PrintValue(result);
            UNPROTECT(1);
        }

        setCurrentExpression(nullptr);

        if (!expr.err_buffer.isEmpty())
        {
            returnCode = ErrorCode;
            returnText = expr.err_buffer;
        }
        else
        {
            returnCode = SuccessCode;
            returnText = expr.std_buffer;
        }
    }
    else
    {
        returnCode = ErrorCode;
        returnText = i18n("Error Parsing Command");
    }

    setCurrentExpression(nullptr);

    // Check for plot files
    QFileInfo f(m_curPlotFile);
    qDebug()<<"RServer: file:" << m_curPlotFile << "exists:" << f.exists() << "size:" << f.size();
    if (f.exists() && f.size() > 0)
    {
        m_expressionFiles << f.filePath();
        // Would call newPlotDevice() here if implementing full plot support
    }

    // Add any other files
    m_expressionFiles << m_expressionFiles;

    sendStatusChanged(0); // Idle
    sendExpressionFinished(returnCode, returnText, m_expressionFiles);
}

void RServer::handleAnswerRequest(QDataStream& stream)
{
    QString answer = RProtocol::readString(stream);
    m_requestCache = answer;
    qDebug() << "Received answer:" << answer;
}

void RServer::sendMessage(quint8 msgType, const QByteArray& data)
{
    if (!m_client || m_client->state() != QLocalSocket::ConnectedState)
        return;

    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    // Message format: length(quint32) | type(quint8) | data
    stream << static_cast<quint32>(data.size() + 1); // +1 for type byte
    stream << msgType;
    stream.writeRawData(data.constData(), data.size());

    m_client->write(message);
    m_client->flush();
}

void RServer::sendStatusChanged(int status)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << static_cast<quint8>(status);

    sendMessage(static_cast<quint8>(RProtocol::ServerMessage::StatusChanged), data);
}

void RServer::sendExpressionFinished(int returnCode, const QString& text, const QStringList& files)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    stream << static_cast<quint8>(returnCode);
    RProtocol::writeString(stream, text);
    RProtocol::writeStringList(stream, files);

    sendMessage(static_cast<quint8>(RProtocol::ServerMessage::ExpressionFinished), data);
}

void RServer::sendInputRequested(const QString& prompt)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    RProtocol::writeString(stream, prompt);

    sendMessage(static_cast<quint8>(RProtocol::ServerMessage::InputRequested), data);
}

QString RServer::requestInput(const QString& prompt)
{
    sendInputRequested(prompt);

    // TODO: Implement proper event loop waiting for answer
    // For now, this is a placeholder
    return m_requestCache;
}

void RServer::addFileToOutput(const QString& file)
{
    m_expressionFiles.append(file);
}

// Keep existing initR() implementation
void RServer::initR()
{
    //Setup environment variables
    #include "renvvars.h"

    for (int i = 0; R_VARS[i] != nullptr; i+= 2)
        qputenv(R_VARS[i], R_VARS[i+1]);

    const char *R_argv[] = {"Cantor", "--no-save", "--no-readline", "", ""};
    const char *R_argv_opt[] = {"--vanilla", "--slave"};
    int R_argc = (sizeof(R_argv) - sizeof(R_argv_opt)) / sizeof(R_argv[0]);

    setupCallbacks(this);

    Rf_initEmbeddedR(R_argc, (char**) R_argv);
    R_ReplDLLinit();

#ifdef Q_OS_WIN
    // On Windows, activate callbacks after R initialization
    connectCallbacks();
#endif

    // Set gui editor for R
    // Note: runCommand not available yet, would need to execute directly
    // For now, skip autorun scripts during initialization
}

void RServer::endR()
{
   Rf_endEmbeddedR(0);
}
