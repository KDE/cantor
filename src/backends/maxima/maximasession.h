/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
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

    enum Mode {Maxima, Lisp};

    explicit MaximaSession(Cantor::Backend*);

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

    Mode mode() const;
    void setMode(Mode);

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

    QProcess* m_process{nullptr};
    QString m_cache;
    bool m_justRestarted{false};
    Mode m_mode{Maxima};
};

#endif /* _MAXIMASESSION_H */
