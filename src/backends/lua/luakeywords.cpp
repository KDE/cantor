/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Sirgienko Nikita <warquark@gmail.com>
*/

#include "luakeywords.h"

#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>

LuaKeywords::LuaKeywords()
{
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("Lua"));

    m_keywords = definition.keywordList(QLatin1String("keywords"));
    m_keywords << definition.keywordList(QLatin1String("control"));

    m_variables = definition.keywordList(QLatin1String("basevar"));

    m_functions = definition.keywordList(QLatin1String("basefunc"));
}

LuaKeywords* LuaKeywords::instance()
{
    static LuaKeywords* inst = nullptr;

    if(inst == nullptr){
        inst = new LuaKeywords();
        std::sort(inst->m_functions.begin(), inst->m_functions.end());
        std::sort(inst->m_keywords.begin(), inst->m_keywords.end());
        std::sort(inst->m_variables.begin(), inst->m_variables.end());
    }

    return inst;
}

const QStringList& LuaKeywords::functions() const
{
    return m_functions;
}

const QStringList& LuaKeywords::keywords() const
{
    return m_keywords;
}

const QStringList& LuaKeywords::variables() const
{
    return m_variables;
}
