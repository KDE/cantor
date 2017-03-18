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
    // Put the lists in alphabetical order
    // Begin m_keywords initialization
    m_keywords << QLatin1String("abstract");
    m_keywords << QLatin1String("baremodule");
    m_keywords << QLatin1String("begin");
    m_keywords << QLatin1String("bitstype");
    m_keywords << QLatin1String("break");
    m_keywords << QLatin1String("catch");
    m_keywords << QLatin1String("const");
    m_keywords << QLatin1String("continue");
    m_keywords << QLatin1String("do");
    m_keywords << QLatin1String("elseif");
    m_keywords << QLatin1String("else");
    m_keywords << QLatin1String("end");
    m_keywords << QLatin1String("export");
    m_keywords << QLatin1String("finally");
    m_keywords << QLatin1String("for");
    m_keywords << QLatin1String("function");
    m_keywords << QLatin1String("global");
    m_keywords << QLatin1String("if");
    m_keywords << QLatin1String("immutable");
    m_keywords << QLatin1String("import");
    m_keywords << QLatin1String("importall");
    m_keywords << QLatin1String("let");
    m_keywords << QLatin1String("local");
    m_keywords << QLatin1String("macro");
    m_keywords << QLatin1String("module");
    m_keywords << QLatin1String("quote");
    m_keywords << QLatin1String("return");
    m_keywords << QLatin1String("try");
    m_keywords << QLatin1String("type");
    m_keywords << QLatin1String("typealias");
    m_keywords << QLatin1String("using");
    m_keywords << QLatin1String("while");
    //Finish m_keywords initialization

    //Begin m_variables initialization
    m_variables << QLatin1String("false");
    m_variables << QLatin1String("Inf");
    m_variables << QLatin1String("NaN");
    m_variables << QLatin1String("nothing");
    m_variables << QLatin1String("true");
    //Finish m_variables initialization

    //Begin m_plotShowingCommands initialization
    m_plotShowingCommands << QLatin1String("contour");
    m_plotShowingCommands << QLatin1String("contourf");
    m_plotShowingCommands << QLatin1String("grid");
    m_plotShowingCommands << QLatin1String("grid3d");
    m_plotShowingCommands << QLatin1String("histogram");
    m_plotShowingCommands << QLatin1String("imshow");
    m_plotShowingCommands << QLatin1String("plot");
    m_plotShowingCommands << QLatin1String("plot3");
    m_plotShowingCommands << QLatin1String("polar");
    m_plotShowingCommands << QLatin1String("polyline");
    m_plotShowingCommands << QLatin1String("polyline3d");
    m_plotShowingCommands << QLatin1String("polymarker");
    m_plotShowingCommands << QLatin1String("polymarker3d");
    m_plotShowingCommands << QLatin1String("scatter");
    m_plotShowingCommands << QLatin1String("scatter3");
    m_plotShowingCommands << QLatin1String("show");
    m_plotShowingCommands << QLatin1String("surface");
    //Finish m_plotShowingCommands initialization
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
    m_removedFunctions = m_functions;
    m_functions.clear();
}
