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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "sagehighlighter.h"
#include "sagekeywords.h"

#include <QTextEdit>

SageHighlighter::SageHighlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    addRule(QRegExp(QLatin1String("[A-Za-z0-9_]+(?=\\()")), functionFormat());

    QStringList keywords;
    //Preprocessor
    keywords = SageKeywords::instance()->keywords();
    //specialvars
    keywords << QLatin1String("None") << QLatin1String("self") << QLatin1String("True") << QLatin1String("true") << QLatin1String("False") << QLatin1String("false")
             << QLatin1String("NotImplemented") << QLatin1String("Ellipsis");
    addKeywords(keywords);

    QStringList builtinFunctions;
    builtinFunctions << QLatin1String("__future__") << QLatin1String("__import__") << QLatin1String("__name__") << QLatin1String("abs")
                            << QLatin1String("all") << QLatin1String("any") << QLatin1String("apply") << QLatin1String("basestring") << QLatin1String("bool")
                            << QLatin1String("buffer") << QLatin1String("callable") << QLatin1String("chr") << QLatin1String("classmethod")
                            << QLatin1String("cmp") << QLatin1String("coerce") << QLatin1String("compile") << QLatin1String("complex") << QLatin1String("delattr")
                            << QLatin1String("dict") << QLatin1String("dir") << QLatin1String("divmod") << QLatin1String("enumerate") << QLatin1String("eval")
                            << QLatin1String("execfile") << QLatin1String("file") << QLatin1String("filter") << QLatin1String("float") << QLatin1String("frozenset")
                            << QLatin1String("getattr") << QLatin1String("globals") << QLatin1String("hasattr") << QLatin1String("hash") << QLatin1String("hex")
                            << QLatin1String("id") << QLatin1String("input") << QLatin1String("int") << QLatin1String("intern") << QLatin1String("isinstance")
                            << QLatin1String("issubclass") << QLatin1String("iter") << QLatin1String("len") << QLatin1String("list") << QLatin1String("locals")
                            << QLatin1String("long") << QLatin1String("map") << QLatin1String("max") << QLatin1String("min") << QLatin1String("object") << QLatin1String("oct")
                            << QLatin1String("open") << QLatin1String("ord") << QLatin1String("pow") << QLatin1String("property") << QLatin1String("range")
                            << QLatin1String("raw_input") << QLatin1String("reduce") << QLatin1String("reload") << QLatin1String("repr") << QLatin1String("reversed")
                            << QLatin1String("round") << QLatin1String("set") << QLatin1String("setattr") << QLatin1String("slice") << QLatin1String("sorted") << QLatin1String("staticmethod")
                            << QLatin1String("str") << QLatin1String("sum") << QLatin1String("super") << QLatin1String("tuple") << QLatin1String("type") << QLatin1String("unichr")
                            << QLatin1String("unicode") << QLatin1String("vars") << QLatin1String("xrange") << QLatin1String("zip");
    addRules(builtinFunctions, functionFormat());

    addRule(QRegExp(QLatin1String("\\S*[a-zA-Z\\-\\_]+\\S*\\.(?!\\d)")), objectFormat());

    QStringList exceptionPatterns;
    exceptionPatterns<< QLatin1String("ArithmeticError") << QLatin1String("AssertionError") << QLatin1String("AttributeError")
                     << QLatin1String("BaseException") << QLatin1String("DeprecationWarning") << QLatin1String("EnvironmentError")
                     << QLatin1String("EOFError") << QLatin1String("Exception") << QLatin1String("FloatingPointError")
                     << QLatin1String("FutureWarning") << QLatin1String("GeneratorExit") << QLatin1String("IOError")
                     << QLatin1String("ImportError") << QLatin1String("ImportWarning") << QLatin1String("IndexError")
                     << QLatin1String("KeyError") << QLatin1String("KeyboardInterrupt") << QLatin1String("LookupError")
                     << QLatin1String("MemoryError") << QLatin1String("NameError") << QLatin1String("NotImplementedError")
                     << QLatin1String("OSError") << QLatin1String("OverflowError") << QLatin1String("PendingDeprecationWarning")
                     << QLatin1String("ReferenceError") << QLatin1String("RuntimeError") << QLatin1String("RuntimeWarning")
                     << QLatin1String("StandardError") << QLatin1String("StopIteration") << QLatin1String("SyntaxError")
                     << QLatin1String("SyntaxWarning") << QLatin1String("SystemError") << QLatin1String("SystemExit")
                     << QLatin1String("TypeError") << QLatin1String("UnboundLocalError") << QLatin1String("UserWarning")
                     << QLatin1String("UnicodeError") << QLatin1String("UnicodeWarning") << QLatin1String("UnicodeEncodeError")
                     << QLatin1String("UnicodeDecodeError") << QLatin1String("UnicodeTranslateError") << QLatin1String("ValueError")
                     << QLatin1String("Warning") << QLatin1String("WindowsError") << QLatin1String("ZeroDivisionError");
    addRules(exceptionPatterns, objectFormat());

    addRule(QRegExp(QLatin1String("\".*\"")), stringFormat());
    addRule(QRegExp(QLatin1String("'.*'")), stringFormat());
    addRule(QRegExp(QLatin1String("#[^\n]*")), commentFormat());
}

SageHighlighter::~SageHighlighter()
{
}
