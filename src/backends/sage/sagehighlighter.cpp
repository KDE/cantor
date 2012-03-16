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

SageHighlighter::SageHighlighter(QTextEdit* edit) : Cantor::DefaultHighlighter(edit)
{
    addRule(QRegExp("[A-Za-z0-9_]+(?=\\()"), functionFormat());

    QStringList keywords;
    //Preprocessor
    keywords = SageKeywords::instance()->keywords();
    //specialvars
    keywords << "None" << "self" << "True" << "true" << "False" << "false"
             << "NotImplemented" << "Ellipsis";
    addKeywords(keywords);

    QStringList builtinFunctions;
    builtinFunctions << "__future__" << "__import__" << "__name__" << "abs"
                            << "all" << "any" << "apply" << "basestring" << "bool"
                            << "buffer" << "callable" << "chr" << "classmethod"
                            << "cmp" << "coerce" << "compile" << "complex" << "delattr"
                            << "dict" << "dir" << "divmod" << "enumerate" << "eval"
                            << "execfile" << "file" << "filter" << "float" << "frozenset"
                            << "getattr" << "globals" << "hasattr" << "hash" << "hex"
                            << "id" << "input" << "int" << "intern" << "isinstance"
                            << "issubclass" << "iter" << "len" << "list" << "locals"
                            << "long" << "map" << "max" << "min" << "object" << "oct"
                            << "open" << "ord" << "pow" << "property" << "range"
                            << "raw_input" << "reduce" << "reload" << "repr" << "reversed"
                            << "round" << "set" << "setattr" << "slice" << "sorted" << "staticmethod"
                            << "str" << "sum" << "super" << "tuple" << "type" << "unichr"
                            << "unicode" << "vars" << "xrange" << "zip";
    addRules(builtinFunctions, functionFormat());

    addRule(QRegExp("\\S*[a-zA-Z\\-\\_]+\\S*\\.(?!\\d)"), objectFormat());

    QStringList exceptionPatterns;
    exceptionPatterns<< "ArithmeticError" << "AssertionError" << "AttributeError"
                     << "BaseException" << "DeprecationWarning" << "EnvironmentError"
                     << "EOFError" << "Exception" << "FloatingPointError"
                     << "FutureWarning" << "GeneratorExit" << "IOError"
                     << "ImportError" << "ImportWarning" << "IndexError"
                     << "KeyError" << "KeyboardInterrupt" << "LookupError"
                     << "MemoryError" << "NameError" << "NotImplementedError"
                     << "OSError" << "OverflowError" << "PendingDeprecationWarning"
                     << "ReferenceError" << "RuntimeError" << "RuntimeWarning"
                     << "StandardError" << "StopIteration" << "SyntaxError"
                     << "SyntaxWarning" << "SystemError" << "SystemExit"
                     << "TypeError" << "UnboundLocalError" << "UserWarning"
                     << "UnicodeError" << "UnicodeWarning" << "UnicodeEncodeError"
                     << "UnicodeDecodeError" << "UnicodeTranslateError" << "ValueError"
                     << "Warning" << "WindowsError" << "ZeroDivisionError";
    addRules(exceptionPatterns, objectFormat());

    addRule(QRegExp("\".*\""), stringFormat());
    addRule(QRegExp("'.*'"), stringFormat());
    addRule(QRegExp("#[^\n]*"), commentFormat());
}

SageHighlighter::~SageHighlighter()
{
}
