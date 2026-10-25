/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _RSERVER_H
#define _RSERVER_H

#include <QObject>
#include <QMap>

class QLocalSocket;
class QLocalServer;

class RServer : public QObject
{
  Q_OBJECT

  public:
    enum ReturnCode {SuccessCode = 0, ErrorCode};
    RServer();
    ~RServer() override;

    void initR();
    void endR();

    QString requestInput(const QString& prompt);
    void addFileToOutput(const QString& file);
    void queuePartialOutput(const QString& text);

  private Q_SLOTS:
    void newConnection();
    void readFromClient();
    void clientDisconnected();
    void flushPartialOutput();

  private:
    void handleRunCommand(QDataStream&);
    void handleAnswerRequest(QDataStream&);
    void sendMessage(quint8 msgType, const QByteArray& data);
    void sendStatusChanged(int status);
    void sendExpressionFinished(int returnCode, const QString& text, const QStringList& files);
    void sendInputRequested(const QString& prompt);
    void newPlotDevice();
    void runInternalCommand(const QString&);
    void listSymbols();

  private:
    bool m_isInitialized{false};
    QString m_requestCache;
    QString m_tmpDir;
    QString m_curPlotFile;
    QString m_partialOutputBuffer;
    QStringList m_expressionFiles;
    QLocalServer* m_server{nullptr};
    QLocalSocket* m_client{nullptr};
    QByteArray m_receiveBuffer;
    QMap<QString, QStringList> m_parsedNamespaceFunctions;
    QMap<QString, QStringList> m_parsedNamespaceConstants;

    // Plot settings (received from client with each command)
    quint8 m_plotFormat{0};   // 0=pdf, 1=svg, 2=png
    double m_plotWidth{12.0}; // cm
    double m_plotHeight{8.0}; // cm
};

#endif /* _RSERVER_H */
