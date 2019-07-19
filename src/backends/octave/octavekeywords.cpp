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

#include "octavekeywords.h"

#include <repository.h>
#include <KF5/KSyntaxHighlighting/Definition>

OctaveKeywords::OctaveKeywords()
{
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::Definition definition = m_repository.definitionForName(QLatin1String("Octave"));

    //TODO: KSyntaxHighlighting provides "keywords", "functions", "forge", "builtin" and "commands".
    //we use "keywords" and "functions" at the moment. decide what to do with "forge", "builtin" and "commands".
    m_keywords = definition.keywordList(QLatin1String("keywords"));

    //KSyntaxHighlighting store this keywords separatly of keywords list, so we add them manually
    m_keywords
        << QLatin1String("function") << QLatin1String("endfunction")
        << QLatin1String("for") << QLatin1String("endfor")
        << QLatin1String("while") << QLatin1String("endwhile")
        << QLatin1String("if") << QLatin1String("endif") << QLatin1String("else")
        << QLatin1String("elseif") << QLatin1String("endswitch")
        << QLatin1String("switch") << QLatin1String("case")
        << QLatin1String("end") << QLatin1String("otherwise");

    m_functions = definition.keywordList(QLatin1String("functions"));
    // https://phabricator.kde.org/D18734
    // OUTOFDATE: Remove after 5.56 KSyntaxHighlighting version
    m_functions
        << QLatin1String("plot") << QLatin1String("semilogx") << QLatin1String("semilogy")
        << QLatin1String("loglog") << QLatin1String("polar") << QLatin1String("contour")
        << QLatin1String("stairs") << QLatin1String("errorbar")  << QLatin1String("sombrero")
        << QLatin1String("hist") << QLatin1String("fplot") << QLatin1String("imshow")
        << QLatin1String("stem") << QLatin1String("stem3") << QLatin1String("scatter")
        << QLatin1String("pie") << QLatin1String("quiver") << QLatin1String("compass")
        << QLatin1String("pareto") << QLatin1String("rose") << QLatin1String("feather")
        << QLatin1String("pcolor") << QLatin1String("area") << QLatin1String("fill")
        << QLatin1String("plotmatrix") << QLatin1String("bar") << QLatin1String("comet")
        /* 3d-plots */
        << QLatin1String("plot3") << QLatin1String("isocaps") << QLatin1String("isonormals")
        << QLatin1String("mesh") << QLatin1String("meshc") << QLatin1String("meshz")
        << QLatin1String("surf") << QLatin1String("surfc") << QLatin1String("surfl")
        << QLatin1String("surfnorm") << QLatin1String("isosurface")
        /* 3d-plots defined by a function */
        << QLatin1String("ezplot3") << QLatin1String("ezmesh") << QLatin1String("ezmeshc")
        << QLatin1String("ezsurf") << QLatin1String("ezsurfc");
}

OctaveKeywords* OctaveKeywords::instance()
{
    static OctaveKeywords* inst = nullptr;

    if(inst == nullptr){
        inst = new OctaveKeywords();
        std::sort(inst->m_functions.begin(), inst->m_functions.end());
        std::sort(inst->m_keywords.begin(), inst->m_keywords.end());
    }

    return inst;
}

const QStringList& OctaveKeywords::functions() const
{
    return m_functions;
}

const QStringList& OctaveKeywords::keywords() const
{
    return m_keywords;
}
