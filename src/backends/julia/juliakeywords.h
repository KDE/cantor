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
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
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
     * @return list of predefined commands, that are capable to show plot
     */
    const QStringList &plotShowingCommands() const
    {
        return m_plotShowingCommands;
    }

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
    QStringList m_plotShowingCommands; //< list of predefined plot showing cmds
    QStringList m_variables; //< list of variables known at the moment
    QStringList m_removedVariables; //< list of variables removed during cleaning
    QStringList m_functions; //< list of known function at the moment
    QStringList m_removedFunctions; //< list of functions removed during cleaning

    // We are hidding constructor and destructor for singleton
    JuliaKeywords() = default;
    ~JuliaKeywords() = default;

    /// Do first load of predefined stuff from keywords.xml
    void loadKeywords();
};
