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
#include <QRegularExpression>

class MaximaExpression;
class MaximaVariableModel;

class MaximaSession : public Cantor::Session
{
  Q_OBJECT
  public:
    static const QRegularExpression MaximaOutputPrompt;
    static const QRegularExpression MaximaInputPrompt;

    explicit MaximaSession( Cantor::Backend* backend);

    void login() override;
    void logout() override;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;

    void interrupt() override;
    void sendInputToProcess(const QString&);

    void setTypesettingEnabled(bool) override;

    Cantor::CompletionObject* completionFor(const QString& command, int index=-1) override;
    Cantor::SyntaxHelpObject* syntaxHelpFor(const QString& command) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject*) override;
    void runFirstExpression() override;

  public Q_SLOTS:
    void readStdOut();
    void readStdErr();

  private Q_SLOTS:
    void currentExpressionChangedStatus(Cantor::Expression::Status);
    void restartMaxima();
    void restartsCooledDown();
    void reportProcessError(QProcess::ProcessError);

  private:
    void write(const QString&);

    QProcess* m_process;
    QString m_cache;
    bool m_justRestarted;
};

#endif /* _MAXIMASESSION_H */
