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
    // Put the keywords list in alphabetical order
    m_keywords << QLatin1String("and") << QLatin1String("as") << QLatin1String("assert") << QLatin1String("break")
               << QLatin1String("class") << QLatin1String("continue") << QLatin1String("def") << QLatin1String("del")
               << QLatin1String("elif") << QLatin1String("else") << QLatin1String("except") << QLatin1String("exec")
               << QLatin1String("finally") << QLatin1String("for") << QLatin1String("from") << QLatin1String("global")
               << QLatin1String("if") << QLatin1String("import") << QLatin1String("in") << QLatin1String("is")
               << QLatin1String("lambda") << QLatin1String("not") << QLatin1String("or") << QLatin1String("pass")
               << QLatin1String("print") << QLatin1String("raise") << QLatin1String("return") << QLatin1String("try")
               << QLatin1String("while") << QLatin1String("with") << QLatin1String("yield");
}

const QStringList& SageKeywords::keywords() const
{
    return m_keywords;
}
