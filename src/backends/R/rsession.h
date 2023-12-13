/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _RSESSION_H
#define _RSESSION_H

#include <QStringList>

#include "session.h"
#include "rserver_interface.h"

class RExpression;
class RVariableModel;
class QProcess;

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
    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;
    void runFirstExpression() override;

    void sendInputToServer(const QString& input);
  protected Q_SLOTS:
    void serverChangedStatus(int status);
    void expressionFinished(int returnCode, const QString& text, const QStringList& files);
    void inputRequested(QString info);

  private:
    QProcess* m_process;
    org::kde::Cantor::R* m_rServer;
};

#endif /* _RSESSION_H */
