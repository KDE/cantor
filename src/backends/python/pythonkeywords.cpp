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

    //Begin m_functions initialization
    m_functions << QLatin1String("__import__");
    m_functions << QLatin1String("abs");
    m_functions << QLatin1String("all");
    m_functions << QLatin1String("any");
    m_functions << QLatin1String("apply");
    m_functions << QLatin1String("basestring");
    m_functions << QLatin1String("bin");
    m_functions << QLatin1String("bool");
    m_functions << QLatin1String("buffer");
    m_functions << QLatin1String("bytearray");
    m_functions << QLatin1String("callable");
    m_functions << QLatin1String("chr");
    m_functions << QLatin1String("classmethod");
    m_functions << QLatin1String("cmp");
    m_functions << QLatin1String("compile");
    m_functions << QLatin1String("complex");
    m_functions << QLatin1String("coerce");
    m_functions << QLatin1String("delattr");
    m_functions << QLatin1String("dict");
    m_functions << QLatin1String("dir");
    m_functions << QLatin1String("divmod");
    m_functions << QLatin1String("enumerate");
    m_functions << QLatin1String("eval");
    m_functions << QLatin1String("execfile");
    m_functions << QLatin1String("file");
    m_functions << QLatin1String("filter");
    m_functions << QLatin1String("float");
    m_functions << QLatin1String("format");
    m_functions << QLatin1String("frozenset");
    m_functions << QLatin1String("getattr");
    m_functions << QLatin1String("globals");
    m_functions << QLatin1String("hasattr");
    m_functions << QLatin1String("hash");
    m_functions << QLatin1String("help");
    m_functions << QLatin1String("hex");
    m_functions << QLatin1String("id");
    m_functions << QLatin1String("input");
    m_functions << QLatin1String("int");
    m_functions << QLatin1String("intern");
    m_functions << QLatin1String("isinstance");
    m_functions << QLatin1String("issubclass");
    m_functions << QLatin1String("iter");
    m_functions << QLatin1String("len");
    m_functions << QLatin1String("list");
    m_functions << QLatin1String("locals");
    m_functions << QLatin1String("long");
    m_functions << QLatin1String("map");
    m_functions << QLatin1String("max");
    m_functions << QLatin1String("memoryview");
    m_functions << QLatin1String("min");
    m_functions << QLatin1String("next");
    m_functions << QLatin1String("object");
    m_functions << QLatin1String("oct");
    m_functions << QLatin1String("open");
    m_functions << QLatin1String("ord");
    m_functions << QLatin1String("pow");
    m_functions << QLatin1String("print");
    m_functions << QLatin1String("property");
    m_functions << QLatin1String("range");
    m_functions << QLatin1String("raw_input");
    m_functions << QLatin1String("reduce");
    m_functions << QLatin1String("reload");
    m_functions << QLatin1String("repr");
    m_functions << QLatin1String("reversed");
    m_functions << QLatin1String("round");
    m_functions << QLatin1String("set");
    m_functions << QLatin1String("setattr");
    m_functions << QLatin1String("slice");
    m_functions << QLatin1String("sorted");
    m_functions << QLatin1String("staticmethod");
    m_functions << QLatin1String("str");
    m_functions << QLatin1String("sum");
    m_functions << QLatin1String("super");
    m_functions << QLatin1String("tuple");
    m_functions << QLatin1String("type");
    m_functions << QLatin1String("unichr");
    m_functions << QLatin1String("unicode");
    m_functions << QLatin1String("vars");
    m_functions << QLatin1String("xrange");
    m_functions << QLatin1String("zip");
    //Finish m_functions initialization

    //Begin m_variables initialization
    m_variables << QLatin1String("False");
    m_variables << QLatin1String("True");
    //Finish m_variables initialization
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
