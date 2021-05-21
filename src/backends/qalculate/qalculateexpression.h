/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QALCULATE_EXPRESSION_H
#define QALCULATE_EXPRESSION_H

#include "expression.h"
#include <vector>
#include <string>
#include <libqalculate/Calculator.h>

#include <QSharedPointer>

class QalculateSession;
class QTemporaryFile;

class QalculateExpression : public Cantor::Expression
{
    Q_OBJECT

private:
    QTemporaryFile *m_tempFile;

    QString m_message;
    enum MsgType { MSG_NONE=0, MSG_INFO=1, MSG_WARN=2, MSG_ERR=4 };

    void evaluatePlotCommand();

    bool stringToBool(const QString&, bool*);
    void deletePlotDataParameters(const std::vector<PlotDataParameters*>&);
    void showMessage(QString msg, MessageType mtype);
    int checkForCalculatorMessages();
    void updateVariables();
    QSharedPointer<PrintOptions> printOptions();
    EvaluationOptions evaluationOptions();
    ParseOptions parseOptions();
    std::string unlocalizeExpression(QString expr);

public:
    explicit QalculateExpression( QalculateSession* session, bool internal = false);
    ~QalculateExpression() override;

    void evaluate() override;
    void interrupt() override;
    void parseOutput(QString& output);
    void parseError(QString& error);
};

#endif
