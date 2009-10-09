/*
    Tims program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMASESSION_H
#define _MAXIMASESSION_H

#include "session.h"
#include "expression.h"

#include <kdirwatch.h>
#include <QRegExp>
#include <QProcess>

class MaximaExpression;
class KProcess;
class QTcpServer;
class QTcpSocket;
class QTimer;

class MaximaSession : public Cantor::Session
{
  Q_OBJECT
  public:
    static const QRegExp MaximaPrompt;
    static const QRegExp MaximaOutputPrompt;

    MaximaSession( Cantor::Backend* backend);
    ~MaximaSession();

    void login();
    void logout();
    void startServer();
    void newMaximaClient(QTcpSocket* socket);
    void newHelperClient(QTcpSocket* socket);

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
    MaximaExpression* evaluateHelperExpression(const QString& command);

    void appendExpressionToQueue(MaximaExpression* expr);
    void appendExpressionToHelperQueue(MaximaExpression* expr);

    void interrupt();
    void interrupt(MaximaExpression* expr);
    void sendInputToProcess(const QString& input);

    void setTypesettingEnabled(bool enable);

    Cantor::TabCompletionObject* tabCompletionFor(const QString& command);
    Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& command);

  public slots:
    void readStdOut();

    void readHelperOut();

  private slots:
    void newConnection();
    void letExpressionParseOutput();
    void currentExpressionChangedStatus(Cantor::Expression::Status status);
    void currentHelperExpressionChangedStatus(Cantor::Expression::Status status);
    void restartMaxima();
    void restartsCooledDown();

    void runFirstExpression();
    void runNextHelperCommand();
    void startHelperProcess();

    void killLabels();

    void reportProcessError(QProcess::ProcessError error);
  private:
    QTcpServer* m_server;
    QTcpSocket* m_maxima;
    KProcess* m_process;
    QTcpSocket* m_helperMaxima;
    KProcess* m_helperProcess; //only used to convert from expression to TeX/get syntax information
    QList<MaximaExpression*> m_expressionQueue;
    QList<MaximaExpression*> m_helperQueue; //Queue used for Expressions that need to be converted to LaTeX
    QString m_cache;

    bool m_isInitialized;
    QString m_tmpPath;

    QTimer* m_restartCooldown;
    bool m_justRestarted;
    bool m_useLegacy;
};

#endif /* _MAXIMASESSION_H */
