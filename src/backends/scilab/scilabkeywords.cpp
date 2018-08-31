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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "scilabkeywords.h"

#include <QStringList>
#include <QXmlStreamReader>
#include <QtAlgorithms>
#include <QDebug>

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

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
        qSort(inst->m_variables);
        qSort(inst->m_functions);
        qSort(inst->m_keywords);
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
