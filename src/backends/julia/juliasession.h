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
#include <QRegularExpression>

#include "session.h"

class JuliaExpression;
class JuliaCompletionObject;
class KProcess;
class QDBusInterface;
namespace Cantor {
    class DefaultVariableModel;
}

/**
 * Implements a Cantor session for the Julia backend
 *
 * It communicates through DBus interface with JuliaServer
 */
class JuliaSession: public Cantor::Session
{
    Q_OBJECT
public:
    /**
     * Constructs session
     *
     * @param backend owning backend
     */
    explicit JuliaSession(Cantor::Backend *backend);

    /**
     * @see Cantor::Session::login
     */
    void login() override;

    /**
     * @see Cantor::Session::logout
     */
    void logout() override;

    /**
     * @see Cantor::Session::interrupt
     */
    void interrupt() override;

    /**
     * @see Cantor::Session::evaluateExpression
     */
    Cantor::Expression *evaluateExpression(
        const QString &command,
        Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete,
        bool internal = false) override;

    /**
     * @see Cantor::Session::completionFor
     */
    Cantor::CompletionObject *completionFor(
        const QString &cmd,
        int index = -1) override;

    /**
     * @see Cantor::Session::syntaxHighlighter
     */
    QSyntaxHighlighter *syntaxHighlighter(QObject *parent) override;

    /**
     * @see Cantor::Session::variableModel
     */
    QAbstractItemModel *variableModel() override;

    /**
     * @return indicator if config says to integrate plots into worksheet
     */
    bool integratePlots();

Q_SIGNALS:
    /**
     * Emit this to update syntax highlighter
     */
    void updateHighlighter();

private Q_SLOTS:
    /**
     * Called when async call to JuliaServer is finished
     */
    void onResultReady();

private:
    KProcess *m_process; //< process to run JuliaServer inside
    QDBusInterface *m_interface; //< interface to JuliaServer

    /// Expressions running at the moment
    QList<JuliaExpression *> m_runningExpressions;
    JuliaExpression *m_currentExpression; //< current expression

    /// Variable management model
    Cantor::DefaultVariableModel *m_variableModel;
    static const QRegularExpression typeVariableInfo;

    /// Cache to speedup modules whos calls
    QMap<QString, QString> m_whos_cache;

    friend JuliaExpression;
    friend JuliaCompletionObject;

    /**
     * Runs Julia expression
     *
     * @param expression expression to run
     */
    void runExpression(JuliaExpression *expression);

    /**
     * Runs Julia piece of code in synchronous mode
     *
     * @param command command to execute
     */
    void runJuliaCommand(const QString &command) const;

    /**
     * Runs Julia piece of code in asynchronous mode. When finished
     * onResultReady is called
     *
     * @param command command to execute
     */
    void runJuliaCommandAsync(const QString &command);

    /**
     * Helper method to get QString returning function result
     *
     * @param method DBus method to call
     * @return result of the method
     */
    QString getStringFromServer(const QString &method);

    /**
     * @return stdout of the last executed command
     */
    QString getOutput();

    /**
     * @return stderr of the last executed command
     */
    QString getError();

    /**
     * @return indicator of exception occurred during the last command execution
     */
    bool getWasException();

    /**
     * Updates variable model by querying all modules in scope with whos command
     */
    void listVariables();
};
