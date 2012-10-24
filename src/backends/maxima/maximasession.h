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
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMASESSION_H
#define _MAXIMASESSION_H

#include "session.h"
#include "expression.h"

#include <kdirwatch.h>
#include <QRegExp>
#include <QProcess>
#include <QXmlStreamReader>

class MaximaExpression;
class MaximaVariableModel;
#ifndef Q_OS_WIN
  class KPtyProcess;
#endif
class KProcess;
class QTcpServer;
class QTimer;
class QAbstractItemModel;

class MaximaSession : public Cantor::Session
{
  Q_OBJECT
  public:
    static const QRegExp MaximaOutputPrompt;

    MaximaSession( Cantor::Backend* backend);
    ~MaximaSession();

    void login();
    void logout();

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);

    void appendExpressionToQueue(MaximaExpression* expr);

    void interrupt();
    void interrupt(MaximaExpression* expr);
    void sendInputToProcess(const QString& input);

    void setTypesettingEnabled(bool enable);

    Cantor::CompletionObject* completionFor(const QString& command, int index=-1);
    Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& command);
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent);
    QAbstractItemModel* variableModel();

  public slots:
    void readStdOut();
    void readStdErr();

  private slots:
    void currentExpressionChangedStatus(Cantor::Expression::Status status);
    void restartMaxima();
    void restartsCooledDown();

    void runFirstExpression();
    void killLabels();

    void reportProcessError(QProcess::ProcessError error);
  private:
//windows doesn't support Pty
#ifdef Q_OS_WIN
    KProcess* m_process;
#else
    KPtyProcess* m_process;
#endif
    QList<MaximaExpression*> m_expressionQueue;
    QString m_cache;
    MaximaVariableModel* m_variableModel;

    enum InitState{NotInitialized, Initializing, Initialized};
    InitState m_initState;
    QString m_tmpPath;

    QTimer* m_restartCooldown;
    bool m_justRestarted;
};

#endif /* _MAXIMASESSION_H */
