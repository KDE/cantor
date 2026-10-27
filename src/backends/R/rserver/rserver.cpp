/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
    SPDX-FileCopyrightText: 2023-2026 by Alexander Semke (alexander.semke@web.de)
*/

#include "rserver.h"
#include "rprotocol.h"
#include "rcallbacks.h"
#include "settings.h"

#include <QScreen>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>

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

RServer::RServer()
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
    QDir(m_tmpDir).removeRecursively();

    if (m_server)
        m_server->close();
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
    qDebug() << "Client disconnected, shutting down";
    m_client->deleteLater();
    m_client = nullptr;
    m_receiveBuffer.clear();
    QCoreApplication::quit();
}

void RServer::handleRunCommand(QDataStream& stream)
{
    QString cmd = RProtocol::readString(stream);
    quint8 internalFlag;
    stream >> internalFlag;
    bool internal = (internalFlag != 0);

    // Read plot settings from client (always current)
    quint8 plotFormat;
    double plotWidth, plotHeight;
    stream >> plotFormat >> plotWidth >> plotHeight;
    qDebug() << "RServer: received plotFormat:" << plotFormat << "size:" << plotWidth << "x" << plotHeight;

    // Recreate plot device if settings changed
    if (plotFormat != m_plotFormat || plotWidth != m_plotWidth || plotHeight != m_plotHeight)
    {
        m_plotFormat = plotFormat;
        m_plotWidth = plotWidth;
        m_plotHeight = plotHeight;
        if (RServerSettings::self()->integratePlots())
            newPlotDevice();
    }

    m_expressionFiles.clear();
    qDebug()<<"RServer: running" << (internal ? "internal " : "") << "command" << cmd;

    sendStatusChanged(1); // Busy

    // Handle special internal commands
    if (internal && cmd == QLatin1String("%model update"))
    {
        listSymbols();
        return;
    }

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

    qDebug() << "RServer: ParseStatus:" << status << "(PARSE_OK=" << PARSE_OK << ")";

    switch (status)
    {
        case PARSE_OK:
            qDebug()<<"RServer: PARSING" << cmd << "went OK";
            for (i = 0; i < Rf_length(cmdexpr); ++i) {
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

        // Check if result is a ggplot object (needs print() to render)
        bool isGgplot = false;
        if (result)
        {
            SEXP classAttr = Rf_getAttrib(result, R_ClassSymbol);
            if (classAttr != R_NilValue)
            {
                for (int i = 0; i < Rf_length(classAttr); i++)
                {
                    const char* className = CHAR(STRING_ELT(classAttr, i));
                    if (strcmp(className, "ggplot") == 0 || strcmp(className, "gg") == 0)
                    {
                        isGgplot = true;
                        break;
                    }
                }
            }
        }

        // Print ggplot objects first (they need print() to render to the device)
        if (result && expr.std_buffer.isEmpty() && isGgplot)
        {
            qDebug()<<"RServer: printing ggplot result...";
            Rf_PrintValue(result);
        }

        setCurrentExpression(nullptr);

        // Now flush plot device and check if a plot was generated
        QString plotFile = m_curPlotFile;
        newPlotDevice();
        QFileInfo f(plotFile);
        // R 4.6+ creates PDF files with ~3.6KB header structure even with no plot.
        // Real plots are larger. Use 4KB threshold for PDF, 100 bytes for others.
        qint64 minSize = plotFile.endsWith(QLatin1String(".pdf")) ? 4000 : 100;
        bool hasPlot = f.exists() && f.size() > minSize;
        qDebug()<<"RServer: file:" << plotFile << "exists:" << f.exists() << "size:" << f.size() << "minSize:" << minSize << "hasPlot:" << hasPlot;
        if (hasPlot)
            m_expressionFiles << plotFile;

        // Print non-ggplot results if no plot was generated and nothing printed yet
        if (result && expr.std_buffer.isEmpty() && !isGgplot && !hasPlot)
        {
            qDebug()<<"RServer: printing result...";
            setCurrentExpression(&expr);  // Re-enable output capture
            SEXP count = PROTECT(R_tryEval(lang2(install("length"), result), nullptr, &errorOccurred));
            if (*INTEGER(count) == 0)
                qDebug()<<"RServer: no result, so show nothing";
            else
                Rf_PrintValue(result);
            UNPROTECT(1);
            setCurrentExpression(nullptr);
        }

        // R uses stderr for progress messages (e.g., package installation), not just errors.
        // Only treat as error if there's no stdout and only stderr, or if R signaled an error.
        if (expr.std_buffer.isEmpty() && !expr.err_buffer.isEmpty())
        {
            // Likely an actual error - no output, only error messages
            returnCode = ErrorCode;
            returnText = expr.err_buffer;
        }
        else
        {
            // Success - show stdout (stderr was shown as progress)
            returnCode = SuccessCode;
            returnText = expr.std_buffer;
        }
    }
    else
    {
        returnCode = ErrorCode;
        returnText = i18n("Error Parsing Command");
    }

    // Flush any pending partial output before sending finished
    flushPartialOutput();

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

void RServer::queuePartialOutput(const QString& text)
{
    m_partialOutputBuffer += text;
    // Flush immediately - R_tryEval blocks so queued invocations won't fire mid-command
    flushPartialOutput();
}

void RServer::flushPartialOutput()
{
    if (m_partialOutputBuffer.isEmpty())
        return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    RProtocol::writeString(stream, m_partialOutputBuffer);

    sendMessage(static_cast<quint8>(RProtocol::ServerMessage::PartialOutput), data);
    m_partialOutputBuffer.clear();
}

QString RServer::requestInput(const QString& prompt)
{
    if (!m_client)
    {
        // Client disconnected, exit R
        qDebug() << "RServer::requestInput: no client, exiting";
        QCoreApplication::quit();
        return QString();
    }

    m_requestCache.clear();
    sendInputRequested(prompt);

    // Wait for answer from client - read directly since we're blocking R's event loop
    // Only process AnswerRequest messages here, leave others in buffer
    while (m_requestCache.isEmpty())
    {
        if (!m_client)
        {
            // Client disconnected while waiting
            qDebug() << "RServer::requestInput: client disconnected, exiting";
            QCoreApplication::quit();
            return QString();
        }

        if (m_client->waitForReadyRead(100))
        {
            QByteArray newData = m_client->readAll();
            qDebug() << "RServer::requestInput: received" << newData.size() << "bytes";

            // Parse just this message - expect AnswerRequest
            if (newData.size() >= 5)
            {
                QDataStream stream(newData);
                stream.setVersion(QDataStream::Qt_6_0);

                quint32 messageLen;
                stream >> messageLen;

                quint8 msgType;
                stream >> msgType;

                qDebug() << "RServer::requestInput: msgType=" << msgType;

                if (static_cast<RProtocol::ClientMessage>(msgType) == RProtocol::ClientMessage::AnswerRequest)
                {
                    handleAnswerRequest(stream);
                }
                else
                {
                    // Not an answer - shouldn't happen, but put back in buffer
                    qWarning() << "RServer::requestInput: unexpected message type" << msgType;
                    m_receiveBuffer.prepend(newData);
                }
            }
        }
    }

    QString result = m_requestCache;
    m_requestCache.clear();
    return result;
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

    // Initialize R first
    Rf_initEmbeddedR(R_argc, (char**) R_argv);
    R_ReplDLLinit();

    // Set up callbacks AFTER R initialization - they get overwritten otherwise
    setupCallbacks(this);
#ifdef Q_OS_WIN
    connectCallbacks();
#endif

    // Log R version info for debugging
    qDebug() << "RServer: R version:" << R_MAJOR << "." << R_MINOR;
    qDebug() << "RServer: R_HOME:" << qgetenv("R_HOME");

    // Set up plot device for inline plots
    qDebug() << "RServer: integratePlots setting:" << RServerSettings::self()->integratePlots();
    if (RServerSettings::self()->integratePlots())
    {
        qDebug() << "RServer: integrating plots";
        newPlotDevice();
    }

    m_isInitialized = true;
}

void RServer::endR()
{
   Rf_endEmbeddedR(0);
}

void RServer::newPlotDevice()
{
    static int deviceNum = 0;

    // Close current device (flushes to disk) before opening new one
    if (m_isInitialized)
        runInternalCommand(QLatin1String("graphics.off()"));

    // Use plot settings received from client
    QString ext, cmd;
    switch (m_plotFormat)
    {
        case 0: // PDF - uses inches (1 inch = 2.54 cm)
            ext = QLatin1String("pdf");
            m_curPlotFile = QString::fromLatin1("%1/Rplot%2.%3").arg(m_tmpDir, QString::number(deviceNum++), ext);
            cmd = QString::fromLatin1("pdf(\"%1\", width = %2, height = %3)")
                .arg(m_curPlotFile)
                .arg(m_plotWidth / 2.54)
                .arg(m_plotHeight / 2.54);
            break;
        case 1: // SVG - uses inches
            ext = QLatin1String("svg");
            m_curPlotFile = QString::fromLatin1("%1/Rplot%2.%3").arg(m_tmpDir, QString::number(deviceNum++), ext);
            cmd = QString::fromLatin1("svg(\"%1\", width = %2, height = %3)")
                .arg(m_curPlotFile)
                .arg(m_plotWidth / 2.54)
                .arg(m_plotHeight / 2.54);
            break;
        default: // PNG - uses pixels
        {
            ext = QLatin1String("png");
            m_curPlotFile = QString::fromLatin1("%1/Rplot%2.%3").arg(m_tmpDir, QString::number(deviceNum++), ext);
            int dpi = 96;
            QScreen* screen = QGuiApplication::primaryScreen();
            if (screen)
                dpi = screen->physicalDotsPerInchX();
            int w = static_cast<int>(m_plotWidth / 2.54 * dpi);
            int h = static_cast<int>(m_plotHeight / 2.54 * dpi);
#ifdef Q_OS_MACOS
            // On macOS, use cairo type for headless operation (quartz requires display)
            cmd = QString::fromLatin1("png(\"%1\", width = %2, height = %3, type = \"cairo\")").arg(m_curPlotFile).arg(w).arg(h);
#else
            cmd = QString::fromLatin1("png(\"%1\", width = %2, height = %3)").arg(m_curPlotFile).arg(w).arg(h);
#endif
            break;
        }
    }

    qDebug() << "RServer: newPlotDevice command:" << cmd;
    runInternalCommand(cmd);
}

void RServer::runInternalCommand(const QString& cmd)
{
    ParseStatus status;
    SEXP cmdSexp, cmdexpr = R_NilValue;
    int errorOccurred;
    QByteArray memBuf = cmd.toUtf8();

    PROTECT(cmdSexp = allocVector(STRSXP, 1));
    SET_STRING_ELT(cmdSexp, 0, mkChar(memBuf.data()));
    cmdexpr = PROTECT(R_ParseVector(cmdSexp, -1, &status, R_NilValue));

    if (status == PARSE_OK) {
        for (int i = 0; i < Rf_length(cmdexpr); ++i) {
            R_tryEval(VECTOR_ELT(cmdexpr, i), nullptr, &errorOccurred);
            if (errorOccurred)
                qDebug() << "RServer: runInternalCommand error in:" << cmd;
        }
    } else {
        qDebug() << "RServer: runInternalCommand parse error:" << status << "for:" << cmd;
    }
    UNPROTECT(2);
}

void RServer::listSymbols()
{
    const QChar recordSep(30);
    const QChar unitSep(31);

    sendStatusChanged(1); // Busy

    QStringList vars, values, funcs, constants;
    int errorOccurred;

    // Get user namespace objects (ls())
    SEXP usr = PROTECT(R_tryEval(lang1(install("ls")), nullptr, &errorOccurred));
    for (int i = 0; i < Rf_length(usr); i++)
    {
        SEXP object = STRING_ELT(usr, i);
        const QString name = QString::fromUtf8(Rf_translateCharUTF8(object));
        SEXP value = Rf_findVar(Rf_installChar(object), R_GlobalEnv);

        if (Rf_isFunction(value))
            funcs << name;
        else if (RServerSettings::variableManagement())
        {
            int convertStatus;
            SEXP valueAsString = PROTECT(R_tryEval(Rf_lang2(Rf_install("toString"), value), nullptr, &convertStatus));
            if (convertStatus == 0)
            {
                vars << name;
                values << QString::fromUtf8(Rf_translateCharUTF8(Rf_asChar(valueAsString)));
            }
            UNPROTECT(1);
        }
        else
            vars << name;
    }
    UNPROTECT(1);

    // Get functions/constants from loaded packages
    SEXP packages = PROTECT(R_tryEval(lang1(install("search")), nullptr, &errorOccurred));
    for (int i = 1; i < Rf_length(packages); i++) // Skip #0 (user env)
    {
        QString packageName = QString::fromUtf8(Rf_translateCharUTF8(STRING_ELT(packages, i)));

        if (!m_parsedNamespaceFunctions.contains(packageName))
        {
            QStringList pkgFuncs, pkgConsts;

            SEXP f = PROTECT(R_tryEval(Rf_lang2(Rf_install("ls"), Rf_ScalarInteger(i + 1)), nullptr, &errorOccurred));
            for (int j = 0; j < Rf_length(f); j++)
            {
                SEXP object = STRING_ELT(f, j);
                const QString name = QString::fromUtf8(Rf_translateCharUTF8(object));
                SEXP value = Rf_installChar(object);

                int err2 = 0;
                // Eval typeof to avoid crash on certain symbols
                SEXP test = PROTECT(R_tryEval(Rf_lang2(Rf_install("typeof"), value), nullptr, &err2));
                Q_UNUSED(test);

                SEXP resultIs = PROTECT(R_tryEval(Rf_lang2(Rf_install("is.function"), value), nullptr, &err2));
                if (QString::fromUtf8(Rf_translateCharUTF8(Rf_asChar(resultIs))) == QLatin1String("TRUE"))
                    pkgFuncs << name;
                else
                    pkgConsts << name;
                UNPROTECT(2);
            }
            UNPROTECT(1);

            m_parsedNamespaceFunctions[packageName] = pkgFuncs;
            m_parsedNamespaceConstants[packageName] = pkgConsts;
        }

        funcs += m_parsedNamespaceFunctions[packageName];
        constants += m_parsedNamespaceConstants[packageName];
    }
    UNPROTECT(1);

    const QString output = vars.join(recordSep) + unitSep + values.join(recordSep) + unitSep + funcs.join(recordSep) + unitSep + constants.join(recordSep);
    sendStatusChanged(0); // Idle
    sendExpressionFinished(SuccessCode, output, QStringList());
}
