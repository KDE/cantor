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

class JuliaKeywords
{
public:
    static JuliaKeywords *instance();

    const QStringList &keywords() const { return m_keywords; }

    const QStringList &variables() const { return m_variables; }
    const QStringList &removedVariables() const { return m_removedVariables; }
    void clearVariables();
    void addVariable(const QString &variable);

    const QStringList &functions() const { return m_functions; }
    const QStringList &removedFunctions() const { return m_removedFunctions; }
    void clearFunctions();
    void addFunction(const QString &function);

private:
    QStringList m_keywords;
    QStringList m_variables;
    QStringList m_removedVariables;
    QStringList m_functions;
    QStringList m_removedFunctions;

    JuliaKeywords() {}
    ~JuliaKeywords() {}

    void loadFromFile();
};
