/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "maximakeywords.h"

#include <QDebug>

#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Definition>

MaximaKeywords* MaximaKeywords::instance()
{
    static MaximaKeywords* inst=nullptr;
    if(inst==nullptr)
    {
        inst=new MaximaKeywords();
        inst->loadKeywords();
    }

    return inst;
}

void MaximaKeywords::loadKeywords()
{
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("Maxima"));
    m_keywords = definition.keywordList(QLatin1String("MaximaKeyword"));
    m_functions = definition.keywordList(QLatin1String("MaximaFunction"));
    m_variables = definition.keywordList(QLatin1String("MaximaVariable"));

    // This is missing in KSyntaxHighlighting.
    // https://phabricator.kde.org/D18714
    // OUTOFDATE: Remove after 5.55 KSyntaxHighlighting version
    m_variables << QLatin1String("%pi") << QLatin1String("%e") << QLatin1String(" %i")
                << QLatin1String("%gamma") << QLatin1String("ind") << QLatin1String("inf")
                << QLatin1String("infinity") << QLatin1String("minf") << QLatin1String("%phi")
                << QLatin1String("und") << QLatin1String("zeroa") << QLatin1String("zerob");

    m_functions << QLatin1String("celine");

    // We use qBinarySearch with this lists
    std::sort(m_keywords.begin(), m_keywords.end());
    std::sort(m_functions.begin(), m_functions.end());
    std::sort(m_variables.begin(), m_variables.end());
}

const QStringList& MaximaKeywords::variables() const
{
    return m_variables;
}

const QStringList& MaximaKeywords::functions() const
{
    return m_functions;
}

const QStringList& MaximaKeywords::keywords() const
{
    return m_keywords;
}
