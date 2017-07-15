/*************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

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
    QalculateExpression( QalculateSession* session);
    ~QalculateExpression();

    void evaluate();
    void interrupt();
    void parseOutput(QString& output);
    void parseError(QString& error);
};

#endif
