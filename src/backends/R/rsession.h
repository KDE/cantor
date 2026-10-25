/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2026 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _RSESSION_H
#define _RSESSION_H

#include "session.h"

class QLocalSocket;
class QProcess;
class QDataStream;

class RSession : public Cantor::Session
{
  Q_OBJECT
  public:
    explicit RSession(Cantor::Backend*);
    ~RSession() override;

    void login() override;
    void logout() override;

    void interrupt() override;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
    void runFirstExpression() override;

    void sendInputToServer(const QString& input);

  Q_SIGNALS:
    void modelUpdateFinished(const QString& result);

  private Q_SLOTS:
    void readFromServer();
    void socketError();

  private:
    void sendMessage(quint8 msgType, const QByteArray& data);
    void handleStatusChanged(QDataStream&);
    void handleExpressionFinished(QDataStream&);
    void handleInputRequested(QDataStream&);
    void handlePartialOutput(QDataStream&);

    QProcess* m_process{nullptr};
    QLocalSocket* m_socket{nullptr};
    QByteArray m_receiveBuffer;
};

#endif /* _RSESSION_H */
