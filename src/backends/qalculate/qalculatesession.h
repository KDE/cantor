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

#ifndef QALCULATE_SESSION_H
#define QALCULATE_SESSION_H

#include "session.h"
#include "qalculateexpression.h"

#include <QSharedPointer>
#include <QQueue>
#include <QMap>

#include <libqalculate/Variable.h>
#include <libqalculate/MathStructure.h>

namespace Cantor {
class DefaultVariableModel;
}

class QalculateEngine;
class QProcess;


class QalculateSession : public Cantor::Session
{
    Q_OBJECT

private:
    Cantor::DefaultVariableModel* m_variableModel;
    QProcess* m_process;
    QalculateExpression* m_currentExpression;
    QString m_output;
    QString m_finalOutput;
    QString m_currentCommand;
    QString m_saveError;
    QQueue<QalculateExpression*> m_expressionQueue;
    QQueue<QString> m_commandQueue;
    bool m_isSaveCommand;


private:
    void runExpressionQueue();
    void runCommandQueue();
    QString parseSaveCommand(QString& currentCmd);
    void storeVariables(QString& currentCmd, QString output);

public:
    QalculateSession( Cantor::Backend* backend);
    ~QalculateSession() override;

    void login() override;
    void logout() override;

    void interrupt() override;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
    Cantor::CompletionObject* completionFor(const QString& cmd, int index=-1) override;
    Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& cmd) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;

    void runExpression();
    QAbstractItemModel* variableModel() override;

public:
    QMap<QString,QString> variables;

public Q_SLOTS:
    void readOutput();
    void readError();
    void processStarted();
    void currentExpressionStatusChanged(Cantor::Expression::Status status);
};

#endif
