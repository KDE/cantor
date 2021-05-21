/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/
#include "sagekeywords.h"

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

#include <QDebug>

SageKeywords* SageKeywords::instance()
{
    static SageKeywords* inst=nullptr;
    if(inst==nullptr)
    {
        inst = new SageKeywords();
        inst->loadKeywords();
    }

    return inst;
}

void SageKeywords::loadKeywords()
{
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("Python"));

    m_keywords << definition.keywordList(QLatin1String("import"));
    m_keywords << definition.keywordList(QLatin1String("defs"));
    m_keywords << definition.keywordList(QLatin1String("operators"));
    m_keywords << definition.keywordList(QLatin1String("flow"));

    m_functions << definition.keywordList(QLatin1String("builtinfuncs"));
    m_functions << definition.keywordList(QLatin1String("overloaders"));

    m_variables << definition.keywordList(QLatin1String("specialvars"));
}

const QStringList& SageKeywords::keywords() const
{
    return m_keywords;
}

const QStringList& SageKeywords::functions() const
{
    return m_functions;
}

const QStringList& SageKeywords::variables() const
{
    return m_variables;
}
