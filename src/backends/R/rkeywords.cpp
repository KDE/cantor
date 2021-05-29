/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Sirgienko Nikita <warquark@gmail.com>
*/

#include "rkeywords.h"

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

RKeywords::RKeywords()
{
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("R Script"));

    m_keywords = definition.keywordList(QLatin1String("controls"));
    m_keywords << definition.keywordList(QLatin1String("words"));
}

RKeywords* RKeywords::instance()
{
    static RKeywords* inst = nullptr;

    if(inst == nullptr){
        inst = new RKeywords();
        std::sort(inst->m_keywords.begin(), inst->m_keywords.end());
    }

    return inst;
}

const QStringList& RKeywords::keywords() const
{
    return m_keywords;
}
