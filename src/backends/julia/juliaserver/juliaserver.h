/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com
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
 * http://docs.julialang.org/en/release-0.4/manual/embedding/ to get results.
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
     * @param path path to julia executable
     */
    Q_SCRIPTABLE void login(const QString &path) const;

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
    QStringList m_functions;
    static QStringList INTERNAL_VARIABLES;
};
