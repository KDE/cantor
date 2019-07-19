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

#include "pythonkeywords.h"

#include <QFile>
#include <QDebug>

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

PythonKeywords::PythonKeywords()
{
    qDebug() << "PythonKeywords constructor";

}

PythonKeywords* PythonKeywords::instance()
{
    static PythonKeywords* inst = nullptr;
    if(inst == nullptr)
    {
        inst = new PythonKeywords();
        inst->loadKeywords();
    }

    return inst;
}

void PythonKeywords::loadKeywords()
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

    // We use qBinarySearch in PythonCompletetionObject for type fetching
    std::sort(m_keywords.begin(), m_keywords.end());
    std::sort(m_functions.begin(), m_functions.end());
    std::sort(m_variables.begin(), m_variables.end());
}

void PythonKeywords::loadFromModule(const QString& module, const QStringList& keywords)
{
    qDebug() << "Module imported" << module;

    if (module.isEmpty()){
        for(int contKeyword = 0; contKeyword < keywords.size(); contKeyword++){
            m_functions << keywords.at(contKeyword);
        }
    } else {
        m_variables << module;

        for(int contKeyword = 0; contKeyword < keywords.size(); contKeyword++){
            m_functions << module + QLatin1String(".") + keywords.at(contKeyword);
        }
    }
}

const QStringList& PythonKeywords::variables() const
{
    return m_variables;
}

const QStringList& PythonKeywords::functions() const
{
    return m_functions;
}

const QStringList& PythonKeywords::keywords() const
{
    return m_keywords;
}
