/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#pragma once

#include <QMap>
#include <QProcess>
#include <QRegularExpression>

#include "session.h"

class JuliaExpression;
class JuliaCompletionObject;
class JuliaVariableModel;
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
    ~JuliaSession() override;

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

    QString plotFilePrefixPath() const;

private Q_SLOTS:
    /**
     * Called when async call to JuliaServer is finished
     */
    void onResultReady();

    // Handler for cantor_juliaserver crashes
    void reportServerProcessError(QProcess::ProcessError serverError);

private:
    KProcess *m_process; //< process to run JuliaServer inside
    QDBusInterface *m_interface; //< interface to JuliaServer

    /// Cache to speedup modules whos calls
    QMap<QString, QString> m_whos_cache;

    /// Variables for handling plot integration: settings value and real state
    QString m_plotFilePrefixPath;
    bool m_isIntegratedPlotsEnabled;
    bool m_isIntegratedPlotsSettingsEnabled;

    void runFirstExpression() override;

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

    void updateGraphicPackagesFromSettings();

    QString graphicPackageErrorMessage(QString packageId) const override;
};
