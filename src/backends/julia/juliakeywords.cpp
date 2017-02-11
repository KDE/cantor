/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliakeywords.h"

#include <QDebug>

JuliaKeywords *JuliaKeywords::instance()
{
    static JuliaKeywords *inst = 0;
    if (inst == 0) {
        inst = new JuliaKeywords();
        inst->loadKeywords();
    }

    return inst;
}

void JuliaKeywords::loadKeywords()
{
    // Put the keywords list in alphabetical order
    m_keywords << QLatin1String("abstract") << QLatin1String("baremodule") << QLatin1String("begin")
               << QLatin1String("bitstype") << QLatin1String("break") << QLatin1String("catch") << QLatin1String("const")
               << QLatin1String("continue") << QLatin1String("do") << QLatin1String("elseif") << QLatin1String("else")
               << QLatin1String("end") << QLatin1String("export") << QLatin1String("finally") << QLatin1String("for")
               << QLatin1String("function") << QLatin1String("global") << QLatin1String("if") << QLatin1String("immutable")
               << QLatin1String("import") << QLatin1String("importall") << QLatin1String("let") << QLatin1String("local")
               << QLatin1String("macro") << QLatin1String("module") << QLatin1String("quote") << QLatin1String("return")
               << QLatin1String("try") << QLatin1String("type") << QLatin1String("typealias") << QLatin1String("using")
               << QLatin1String("while");

    m_variables << QLatin1String("false") << QLatin1String("Inf") << QLatin1String("NaN") << QLatin1String("nothing")
                << QLatin1String("true");

    m_plotShowingCommands << QLatin1String("contour") << QLatin1String("contourf") << QLatin1String("grid")
                          << QLatin1String("grid3d") << QLatin1String("histogram") << QLatin1String("imshow")
                          << QLatin1String("plot") << QLatin1String("plot3") << QLatin1String("polar")
                          << QLatin1String("polyline") << QLatin1String("polyline3d") << QLatin1String("polymarker")
                          << QLatin1String("polymarker3d") << QLatin1String("scatter") << QLatin1String("scatter3")
                          << QLatin1String("show") << QLatin1String("surface");
}

void JuliaKeywords::addVariable(const QString &variable)
{
    if (!m_variables.contains(variable)) {
        m_variables << variable;
    }
}

void JuliaKeywords::clearVariables()
{
    m_removedVariables = m_variables;
    m_variables.clear();
}

void JuliaKeywords::addFunction(const QString &function)
{
    if (!m_functions.contains(function)) {
        m_functions << function;
    }
}

void JuliaKeywords::clearFunctions()
{
    m_removedFunctions == m_functions;
    m_functions.clear();
}
