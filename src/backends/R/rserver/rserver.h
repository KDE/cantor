/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2025 The Cantor Team
*/

#ifndef _RSERVER_H
#define _RSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QString>
#include <QStringList>

class RServer : public QObject
{
  Q_OBJECT

  public:
    enum ReturnCode { SuccessCode=0, ErrorCode};
    RServer();
    ~RServer() override;

    void initR();
    void endR();

    QString requestInput(const QString& prompt);
    void addFileToOutput(const QString& file);

  private Q_SLOTS:
    void newConnection();
    void readFromClient();
    void clientDisconnected();

  private:
    void handleRunCommand(QDataStream& stream);
    void handleAnswerRequest(QDataStream& stream);
    void sendMessage(quint8 msgType, const QByteArray& data);
    void sendStatusChanged(int status);
    void sendExpressionFinished(int returnCode, const QString& text, const QStringList& files);
    void sendInputRequested(const QString& prompt);
    void newPlotDevice();
    void runInternalCommand(const QString& cmd);

  private:
    bool m_isInitialized;
    QString m_requestCache;
    QString m_tmpDir;
    QString m_curPlotFile;
    QStringList m_expressionFiles;
    QLocalServer* m_server;
    QLocalSocket* m_client;
    QByteArray m_receiveBuffer;
};

#endif /* _RSERVER_H */
