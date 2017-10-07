/*
    This program is free software; you can redistribute it and/or
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

#ifndef _RSESSION_H
#define _RSESSION_H

#include <QRegExp>
#include <QStringList>

#include "session.h"


class RExpression;
class QProcess;

class RSession : public Cantor::Session
{
    Q_OBJECT
public:
    RSession( Cantor::Backend* backend);
    ~RSession();

    void login();
    void logout();

    void interrupt();

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
    Cantor::CompletionObject* completionFor(const QString& command, int index=-1);
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent);

    void runExpression(RExpression* expr);

protected Q_SLOTS:
    void receiveSymbols(const QStringList& v, const QStringList & f);
    void fillSyntaxRegExps(QVector<QRegExp>& v, QVector<QRegExp>& f);

Q_SIGNALS:
    void symbolsChanged();


public Q_SLOTS:
    void readOutput();
    void readError();
    void processStarted();
    void currentExpressionStatusChanged(Cantor::Expression::Status status);

private:
    QProcess* m_Process;
    RExpression* m_CurrentExpression;
    QString m_Output;
    QString m_Error;
    /* Available variables and functions, TODO make full classes and type info */
    QStringList m_variables;
    QStringList m_functions;
};

#endif /* _RSESSION_H */
