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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximakeywords.h"

#include <QDebug>

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

MaximaKeywords::MaximaKeywords()
{

}


MaximaKeywords::~MaximaKeywords()
{

}

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
