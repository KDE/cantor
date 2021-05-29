/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
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
