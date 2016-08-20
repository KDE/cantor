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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#pragma once

#include <QMap>

#include "session.h"

class JuliaExpression;
class JuliaCompletionObject;
class KProcess;
class QDBusInterface;
namespace Cantor {
    class DefaultVariableModel;
}

class JuliaSession: public Cantor::Session
{
    Q_OBJECT
public:
    JuliaSession(Cantor::Backend *backend);
    virtual ~JuliaSession() {}

    virtual void login() override;
    virtual void logout() override;

    virtual void interrupt() override;

    virtual Cantor::Expression *evaluateExpression(
        const QString &command,
        Cantor::Expression::FinishingBehavior behave) override;

    virtual Cantor::CompletionObject *completionFor(
        const QString &cmd,
        int index = -1) override;

    virtual QSyntaxHighlighter *syntaxHighlighter(QObject *parent);
    virtual QAbstractItemModel *variableModel() override;

Q_SIGNALS:
    void updateHighlighter();

private Q_SLOTS:
    void onResultReady();

private:
    KProcess *m_process;
    QDBusInterface *m_interface;

    QList<JuliaExpression *> m_runningExpressions;
    JuliaExpression *m_currentExpression;

    Cantor::DefaultVariableModel *m_variableModel;

    QMap<QString, QString> m_whos_cache;

    friend JuliaExpression;
    friend JuliaCompletionObject;

    void runExpression(JuliaExpression *expression);

    void runJuliaCommand(const QString &command) const;
    void runJuliaCommandAsync(const QString &command);

    QString getStringFromServer(const QString &method);
    QString getOutput();
    QString getError();
    bool getWasException();

    void listVariables();
};
