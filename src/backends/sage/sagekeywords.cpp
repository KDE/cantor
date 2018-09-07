/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
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
