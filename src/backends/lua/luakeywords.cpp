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
    Copyright (C) 2018 Sirgienko Nikita <warquark@gmail.com>
*/

#include "luakeywords.h"

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

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
        qSort(inst->m_functions);
        qSort(inst->m_keywords);
        qSort(inst->m_variables);
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