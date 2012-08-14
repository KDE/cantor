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

#ifndef _SAGESESSION_H
#define _SAGESESSION_H

#include "session.h"
#include "expression.h"

#include <kdirwatch.h>
#include <QProcess>

class SageExpression;
class KPtyProcess;


class SageSession : public Cantor::Session
{
  Q_OBJECT
  public:
    static const QByteArray SagePrompt;
    static const QByteArray SageAlternativePrompt;

    SageSession( Cantor::Backend* backend);
    ~SageSession();

    void login();
    void logout();

    Cantor::Expression* evaluateExpression(const QString& command,Cantor::Expression::FinishingBehavior behave);

    void appendExpressionToQueue(SageExpression* expr);

    void interrupt();

    void sendSignalToProcess(int signal);
    void sendInputToProcess(const QString& input);
    void waitForNextPrompt();

    void setTypesettingEnabled(bool enable);

    Cantor::CompletionObject* completionFor(const QString& command, int index=-1);
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent);


  public slots:
    void readStdOut();
    void readStdErr();

  private slots:
    void currentExpressionChangedStatus(Cantor::Expression::Status status);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void reportProcessError(QProcess::ProcessError error);
    void fileCreated(const QString& path);

  private:
    void runFirstExpression();
  private:
    KPtyProcess* m_process;
    QList<SageExpression*> m_expressionQueue;
    bool m_isInitialized;
    QString m_tmpPath;
    KDirWatch m_dirWatch;
    bool m_waitingForPrompt;
};

#endif /* _SAGESESSION_H */
