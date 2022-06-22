/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#include "scilabkeywords.h"

#include <QXmlStreamReader>
#include <QtAlgorithms>
#include <QDebug>

#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>

ScilabKeywords::ScilabKeywords()
{
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("scilab"));

    m_keywords << definition.keywordList(QLatin1String("Structure-keywords"));
    m_keywords << definition.keywordList(QLatin1String("Control-keywords"));
    m_keywords << definition.keywordList(QLatin1String("Function-keywords"));
    m_keywords << definition.keywordList(QLatin1String("Warning-keywords"));
    m_keywords << definition.keywordList(QLatin1String("Function-keywords"));

    //TODO: This keywords missing in scilab syntax file
    m_keywords << QLatin1String("case") << QLatin1String("catch") << QLatin1String("continue");
    m_keywords << QLatin1String("try");

    m_functions << definition.keywordList(QLatin1String("functions"));

    //TODO: Should we use this keywordList as variables?
    m_variables << definition.keywordList(QLatin1String("Constants-keyword"));
}

ScilabKeywords* ScilabKeywords::instance()
{
    static ScilabKeywords* inst = nullptr;

    if(inst == nullptr){
        inst = new ScilabKeywords();
        std::sort(inst->m_variables.begin(), inst->m_variables.end());
        std::sort(inst->m_functions.begin(), inst->m_functions.end());
        std::sort(inst->m_keywords.begin(), inst->m_keywords.end());
    }

    return inst;
}

const QStringList& ScilabKeywords::variables() const
{
    return m_variables;
}

const QStringList& ScilabKeywords::functions() const
{
    return m_functions;
}

const QStringList& ScilabKeywords::keywords() const
{
    return m_keywords;
}
