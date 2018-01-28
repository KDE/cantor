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
#include <QProcess>

class MaximaExpression;
class MaximaVariableModel;

#ifdef Q_OS_WIN
  class KProcess;
#else
  class KPtyProcess;
#endif

class MaximaSession : public Cantor::Session
{
  Q_OBJECT
  public:
    static const QRegExp MaximaOutputPrompt;

    MaximaSession( Cantor::Backend* backend);

    void login() Q_DECL_OVERRIDE;
    void logout() Q_DECL_OVERRIDE;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave) Q_DECL_OVERRIDE;

    void appendExpressionToQueue(MaximaExpression* expr);

    void interrupt() Q_DECL_OVERRIDE;
    void interrupt(MaximaExpression* expr);
    void sendInputToProcess(const QString& input);

    void setTypesettingEnabled(bool enable) Q_DECL_OVERRIDE;

    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) Q_DECL_OVERRIDE;
    Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& command) Q_DECL_OVERRIDE;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) Q_DECL_OVERRIDE;
    QAbstractItemModel* variableModel() Q_DECL_OVERRIDE;

  public Q_SLOTS:
    void readStdOut();
    void readStdErr();

  private Q_SLOTS:
    void currentExpressionChangedStatus(Cantor::Expression::Status status);
    void restartMaxima();
    void restartsCooledDown();

    void runFirstExpression();
    void killLabels();

    void reportProcessError(QProcess::ProcessError error);

  private:
    void write(const QString&);

    QProcess* m_process;
    QList<MaximaExpression*> m_expressionQueue;
    QString m_cache;
    MaximaVariableModel* m_variableModel;
    bool m_justRestarted;
};

#endif /* _MAXIMASESSION_H */
