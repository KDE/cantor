/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com
*/
#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>

#include <julia.h>

/**
 * Implementation of command execution server with DBus interface for Julia
 * language.
 *
 * Uses Julia embedding
 * https://docs.julialang.org/en/v1/manual/embedding/ to get results.
 */
class JuliaServer: public QObject
{
    Q_OBJECT
public:
    explicit JuliaServer(QObject *parent = nullptr);

    ~JuliaServer() override;

public Q_SLOTS:
    /**
     * Initializer for JuliaServer. Call this first before using it
     *
     * @return 0 - if all OK, 1 - if sys.so file missing. For error 1 - the filename of the missing file can be requested via getError()
     * @param path path to julia executable
     */
    Q_SCRIPTABLE int login(const QString &path);

    /**
     * Runs a piece of julia code. After this returns use getOutput, getError,
     * getWasException methods to retrieve execution result.
     *
     * @param command maybe multiline piece of julia code to run
     */
    Q_SCRIPTABLE void runJuliaCommand(const QString &command);

    /**
     * @return stdout output of the last command execution
     */
    Q_SCRIPTABLE QString getOutput() const;

    /**
     * @return stderr output of the last command execution
     */
    Q_SCRIPTABLE QString getError() const;

    /**
     * @return indicator that exception was triggered during last command
     *         execution
     */
    Q_SCRIPTABLE bool getWasException() const;

    /**
     * Reparse internal julia module and update list of variables and functions
     *
     * @param variableManagement true, if Variable Management enabled for this session
     */
    Q_SCRIPTABLE void parseModules(bool variableManagement);

    /**
     * @return list of variables in internal Julia's module
     */
    Q_SCRIPTABLE QStringList variablesList();

    /**
     * @return corresponding list of values for variables from variablesList.
     */
    Q_SCRIPTABLE QStringList variableValuesList();

    /**
     * @return corresponding list of values for variables from variablesList.
     */
    Q_SCRIPTABLE QStringList variableSizesList();

    /**
     * @return list of function in internal Julia's module
     */
    Q_SCRIPTABLE QStringList functionsList();

private:
    void parseJlModule(jl_module_t* module, bool parseValue);

    QString fromJuliaString(const jl_value_t* value);
private:
    QString m_error; //< Stores last stderr output
    QString m_output; //< Stores last stdout output
    bool m_was_exception; //< Stores indicator of exception
    QStringList parsedModules;
    QStringList m_variables;
    QStringList m_variableValues;
    QStringList m_variableSize;
    QStringList m_functions;
    static QStringList INTERNAL_VARIABLES;
};
