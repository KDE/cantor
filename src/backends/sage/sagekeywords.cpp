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

#include <QDebug>

SageKeywords::SageKeywords()
{

}

SageKeywords::~SageKeywords()
{

}

SageKeywords* SageKeywords::instance()
{
    static SageKeywords* inst=0;
    if(inst==0)
    {
        inst = new SageKeywords();
        inst->loadKeywords();
    }

    return inst;
}

void SageKeywords::loadKeywords()
{
    //Begin m_keywords initialization
    m_keywords << QLatin1String("and");
    m_keywords << QLatin1String("as");
    m_keywords << QLatin1String("assert");
    m_keywords << QLatin1String("break");
    m_keywords << QLatin1String("class");
    m_keywords << QLatin1String("continue");
    m_keywords << QLatin1String("def");
    m_keywords << QLatin1String("del");
    m_keywords << QLatin1String("elif");
    m_keywords << QLatin1String("else");
    m_keywords << QLatin1String("except");
    m_keywords << QLatin1String("exec");
    m_keywords << QLatin1String("finally");
    m_keywords << QLatin1String("for");
    m_keywords << QLatin1String("from");
    m_keywords << QLatin1String("global");
    m_keywords << QLatin1String("if");
    m_keywords << QLatin1String("import");
    m_keywords << QLatin1String("in");
    m_keywords << QLatin1String("is");
    m_keywords << QLatin1String("lambda");
    m_keywords << QLatin1String("not");
    m_keywords << QLatin1String("or");
    m_keywords << QLatin1String("pass");
    m_keywords << QLatin1String("print");
    m_keywords << QLatin1String("raise");
    m_keywords << QLatin1String("return");
    m_keywords << QLatin1String("try");
    m_keywords << QLatin1String("while");
    m_keywords << QLatin1String("with");
    m_keywords << QLatin1String("yield");
    //Finish m_keywords initialization
}

const QStringList& SageKeywords::keywords() const
{
    return m_keywords;
}
