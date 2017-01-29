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

PythonKeywords::PythonKeywords()
{
    qDebug() << "PythonKeywords construtor";

}


PythonKeywords::~PythonKeywords()
{

}

PythonKeywords* PythonKeywords::instance()
{
    static PythonKeywords* inst = 0;
    if(inst == 0)
    {
        inst = new PythonKeywords();
        inst->loadKeywords();
    }

    return inst;
}

void PythonKeywords::loadKeywords()
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

    m_functions << QLatin1String("__import__") << QLatin1String("abs") << QLatin1String("all") << QLatin1String("any")
                << QLatin1String("apply") << QLatin1String("basestring") << QLatin1String("bin") << QLatin1String("bool")
                << QLatin1String("buffer") << QLatin1String("bytearray") << QLatin1String("callable") << QLatin1String("chr")
                << QLatin1String("classmethod") << QLatin1String("cmp") << QLatin1String("compile") << QLatin1String("complex")
                << QLatin1String("coerce") << QLatin1String("delattr") << QLatin1String("dict") << QLatin1String("dir")
                << QLatin1String("divmod") << QLatin1String("enumerate") << QLatin1String("eval") << QLatin1String("execfile")
                << QLatin1String("file") << QLatin1String("filter") << QLatin1String("float") << QLatin1String("format")
                << QLatin1String("frozenset") << QLatin1String("getattr") << QLatin1String("globals")
                << QLatin1String("hasattr") << QLatin1String("hash") << QLatin1String("help") << QLatin1String("hex")
                << QLatin1String("id") << QLatin1String("input") << QLatin1String("int") << QLatin1String("intern")
                << QLatin1String("isinstance") << QLatin1String("issubclass") << QLatin1String("iter") << QLatin1String("len")
                << QLatin1String("list") << QLatin1String("locals") << QLatin1String("long") << QLatin1String("map")
                << QLatin1String("max") << QLatin1String("memoryview") << QLatin1String("min") << QLatin1String("next")
                << QLatin1String("object") << QLatin1String("oct") << QLatin1String("open") << QLatin1String("ord")
                << QLatin1String("pow") << QLatin1String("print") << QLatin1String("property") << QLatin1String("range")
                << QLatin1String("raw_input") << QLatin1String("reduce") << QLatin1String("reload") << QLatin1String("repr")
                << QLatin1String("reversed") << QLatin1String("round") << QLatin1String("set") << QLatin1String("setattr")
                << QLatin1String("slice") << QLatin1String("sorted") << QLatin1String("staticmethod") << QLatin1String("str")
                << QLatin1String("sum") << QLatin1String("super") << QLatin1String("tuple") << QLatin1String("type")
                << QLatin1String("unichr") << QLatin1String("unicode") << QLatin1String("vars") << QLatin1String("xrange")
                << QLatin1String("zip");

    m_variables << QLatin1String("False") << QLatin1String("True");
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

void PythonKeywords::addVariable(const QString& variable)
{
    qDebug() << "Variable added" << variable;

    if (!m_variables.contains(variable)){
        m_variables << variable;
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
