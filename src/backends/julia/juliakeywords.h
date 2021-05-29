/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#pragma once

#include <QStringList>

/**
 * Keywords storage for Julia session
 *
 * Class is implemented with singleton pattern
 */
class JuliaKeywords
{
public:
    /**
     * @return singleton instance pointer
     */
    static JuliaKeywords *instance();

    /**
     * @return list of Julia language predefined keywords
     */
    const QStringList &keywords() const { return m_keywords; }

    /**
     * @return list of known variable names
     */
    const QStringList &variables() const { return m_variables; }

    /**
     * @return list of variables removed during the last clearVariables() call
     */
    const QStringList &removedVariables() const { return m_removedVariables; }

    /**
     * Clears all known variables
     */
    void clearVariables();

    /**
     * Add new variable to the known list
     *
     * @param variable name of the variable to add
     */
    void addVariable(const QString &variable);

    /**
     * @return list of known function names
     */
    const QStringList &functions() const { return m_functions; }

    /**
     * @return list of functions removed during the last clearFunctions() call
     */
    const QStringList &removedFunctions() const { return m_removedFunctions; }

    /**
     * Clears all known functions
     */
    void clearFunctions();

    /**
     * Add new function to the known list
     *
     * @param function name of the function to add
     */
    void addFunction(const QString &function);

private:
    QStringList m_keywords; //< list of predefined keywords
    QStringList m_variables; //< list of variables known at the moment
    QStringList m_removedVariables; //< list of variables removed during cleaning
    QStringList m_functions; //< list of known function at the moment
    QStringList m_removedFunctions; //< list of functions removed during cleaning

    // We are hiding constructor and destructor for singleton
    JuliaKeywords();
    ~JuliaKeywords() = default;
};
