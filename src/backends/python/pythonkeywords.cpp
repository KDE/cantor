/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
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
