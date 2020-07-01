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
#include "juliakeywords.h"

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

JuliaKeywords::JuliaKeywords()
{
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("Julia"));

    m_keywords = definition.keywordList(QLatin1String("block_begin"));
    m_keywords << definition.keywordList(QLatin1String("block_eb"));
    m_keywords << definition.keywordList(QLatin1String("block_end"));
    m_keywords << definition.keywordList(QLatin1String("keywords"));

    //TODO: Upstream pull request to julia.xml from KSyntaxHighlighting?
    // https://bugs.kde.org/show_bug.cgi?id=403901
    // Add new list to julia.syntax with constans?
    m_variables << QLatin1String("false");
    m_variables << QLatin1String("Inf");
    m_variables << QLatin1String("NaN");
    m_variables << QLatin1String("nothing");
    m_variables << QLatin1String("true");
}

JuliaKeywords *JuliaKeywords::instance()
{
    static JuliaKeywords *inst = nullptr;
    if (inst == nullptr) {
        inst = new JuliaKeywords();
    }

    return inst;
}

void JuliaKeywords::addVariable(const QString &variable)
{
    if (!m_variables.contains(variable)) {
        m_variables << variable;
    }
}

void JuliaKeywords::clearVariables()
{
    m_removedVariables = m_variables;
    m_variables.clear();
}

void JuliaKeywords::addFunction(const QString &function)
{
    if (!m_functions.contains(function)) {
        m_functions << function;
    }
}

void JuliaKeywords::clearFunctions()
{
    m_removedFunctions = m_functions;
    m_functions.clear();
}
