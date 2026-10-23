/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _RSESSION_H
#define _RSESSION_H

#include <QStringList>
#include <QLocalSocket>

#include "session.h"

class RExpression;
class RVariableModel;
class QProcess;
class QDataStream;

namespace Cantor {
  class DefaultVariableModel;
}

class RSession : public Cantor::Session
{
  Q_OBJECT
  public:
    explicit RSession( Cantor::Backend* backend);
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
    void handleStatusChanged(QDataStream& stream);
    void handleExpressionFinished(QDataStream& stream);
    void handleInputRequested(QDataStream& stream);

    QProcess* m_process;
    QLocalSocket* m_socket;
    QByteArray m_receiveBuffer;
};

#endif /* _RSESSION_H */
