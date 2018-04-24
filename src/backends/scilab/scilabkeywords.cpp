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

ScilabKeywords::ScilabKeywords()
{
    qDebug() << "ScilabKeywords construtor";
}


ScilabKeywords::~ScilabKeywords()
{

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

void ScilabKeywords::setupKeywords(QString keywords)
{
    qDebug() << "start parse Scilab keywords";
    QStringList key;
    key = keywords.replace(QLatin1String(" !"), QLatin1String("\n")).replace(QLatin1String("!"), QLatin1String(""))
              .replace(QLatin1String(" "), QLatin1String("")).split(QLatin1String("\n"));

    for(int count = key.indexOf(QLatin1String("(1)")); key.at(count) != QLatin1String("(2)"); count++){
        if(key.at(count) == QLatin1String("")){
            continue;
        }

        qDebug() << key.at(count);
        m_functions << key.at(count);
    }

    for(int count = key.indexOf(QLatin1String("(2)")); key.at(count) != QLatin1String("(3)"); count++){
        if(key.at(count) == QLatin1String("")){
            continue;
        }

        qDebug() << key.at(count);
        m_keywords << key.at(count);
    }

    for(int count = key.indexOf(QLatin1String("(3)")); key.at(count) != QLatin1String("(4)"); count++){
        if(key.at(count) == QLatin1String("")){
            continue;
        }

        qDebug() << key.at(count);
        m_variables << key.at(count);
    }

    for(int count = key.indexOf(QLatin1String("(4)")); count < key.size(); count++){
        if(key.at(count) == QLatin1String("")){
            continue;
        }

        qDebug() << key.at(count);
        m_functions << key.at(count);
    }

    qDebug() << "finish parse scilab keywords";
}

void ScilabKeywords::addVariable(QString variable)
{
    m_variables << variable;
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
