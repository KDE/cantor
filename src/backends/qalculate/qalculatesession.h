/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QALCULATE_SESSION_H
#define QALCULATE_SESSION_H

#include "session.h"
#include "qalculateexpression.h"

#include <QQueue>
#include <QMap>

class QProcess;

class QalculateSession : public Cantor::Session
{
    Q_OBJECT

private:
    QProcess* m_process{nullptr};
    QalculateExpression* m_currentExpression{nullptr};
    QString m_output;
    QString m_finalOutput;
    QString m_currentCommand;
    QString m_saveError;
    QQueue<QalculateExpression*> m_expressionQueue;
    QQueue<QString> m_commandQueue;
    bool m_isSaveCommand{false};

private:
    void runExpressionQueue();
    void runCommandQueue();
    QString parseSaveCommand(QString& currentCmd);
    void storeVariables(QString& currentCmd, QString output);

public:
    explicit QalculateSession(Cantor::Backend*);
    ~QalculateSession() override;

    void login() override;
    void logout() override;

    void interrupt() override;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
    void runExpression();
    const QMap<QString,QString>& getVariables() const;

public:
    QMap<QString,QString> variables;

public Q_SLOTS:
    void readOutput();
    void readError();
    void processStarted();
    void currentExpressionStatusChanged(Cantor::Expression::Status status);
};

#endif
