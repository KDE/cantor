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

#include <QTextEdit>

SageHighlighter::SageHighlighter(QTextEdit* edit) : Cantor::DefaultHighlighter(edit)
{
    HighlightingRule rule;

    //initialize the different formats used to highlight
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);

    builtinFuncFormat.setForeground(Qt::darkGreen);
    builtinFuncFormat.setFontWeight(QFont::Bold);

    exceptionFormat.setForeground(Qt::darkMagenta);
    exceptionFormat.setFontWeight(QFont::Bold);

    objectFormat.setFontWeight(QFont::Bold);

    quotationFormat.setForeground(Qt::darkGreen);

    singleLineCommentFormat.setForeground(Qt::red);

    //Setup the highlighting rules
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    m_highlightingRules.append(rule);

    QStringList keywordPatterns;
    keywordPatterns //Preprocessor
                    << "\\bimport\\b" << "\\bfrom\\b" << "\\bas\\b"
                    //defs
                    << "\\bclass\\b" << "\\bdef\\b" << "\\bdel\\b"
                    << "\\bglobal\\b" << "\\blambda\\b"
                    //operators
                    <<"\\band\\b" << "\\bassert\\b" << "\\bin\\b" << "\\bis\\b"
                    <<"\\bnot\\b" << "\\bor\\b"
                    //flow
                    <<"\\bbeak\\b" << "\\bcontinue\\b" << "\\belif\\b" << "\\belse\\b"
                    <<"\\bexcept\\b" << "\\bfinally\\b" << "\\bfor\\b" << "\\bif\\b"
                    <<"\\bpass\\b" << "\\braise\\b" << "\\breturn\\b" << "\\btry\\b"
                    <<"\\bwhile\\b" << "\\byield\\b"
                    //specialvars
                    << "\\bNone\\b" << "\\bself\\b" << "\\b[Tt]rue\\b" << "\\b[Ff]alse\\b"
                    << "\\bNotImplemented\\b" << "\\bEllipsis\\b";


    foreach (const QString &pattern,  keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        m_highlightingRules.append(rule);
    }

    QStringList builtinFunctionPatterns;
    builtinFunctionPatterns << "\\b__future__\\b" << "\\b__import__\\b" << "\\b__name__\\b" << "\\babs\\b"
                            << "\\ball\\b" << "\\bany\\b" << "\\bapply\\b" << "\\bbasestring\\b" << "\\bbool\\b"
                            << "\\bbuffer\\b" << "\\bcallable\\b" << "\\bchr\\b" << "\\bclassmethod\\b"
                            << "\\bcmp\\b" << "\\bcoerce\\b" << "\\bcompile\\b" << "\\bcomplex\\b" << "\\bdelattr\\b"
                            << "\\bdict\\b" << "\\bdir\\b" << "\\bdivmod\\b" << "\\benumerate\\b" << "\\beval\\b"
                            << "\\bexecfile\\b" << "\\bfile\\b" << "\\bfilter\\b" << "\\bfloat\\b" << "\\bfrozenset\\b"
                            << "\\bgetattr\\b" << "\\bglobals\\b" << "\\bhasattr\\b" << "\\bhash\\b" << "\\bhex\\b"
                            << "\\bid\\b" << "\\binput\\b" << "\\bint\\b" << "\\bintern\\b" << "\\bisinstance\\b"
                            << "\\bissubclass\\b" << "\\biter\\b" << "\\blen\\b" << "\\blist\\b" << "\\blocals\\b"
                            << "\\blong\\b" << "\\bmap\\b" << "\\bmax\\b" << "\\bmin\\b" << "\\bobject\\b" << "\\boct\\b"
                            << "\\bopen\\b" << "\\bord\\b" << "\\bpow\\b" << "\\bproperty\\b" << "\\brange\\b"
                            << "\\braw_input\\b" << "\\breduce\\b" << "\\breload\\b" << "\\brepr\\b" << "\\breversed\\b"
                            << "\\bround\\b" << "\\bset\\b" << "\\bsetattr\\b" << "\\bslice\\b" << "\\bsorted\\b" << "\\bstaticmethod\\b"
                            << "\\bstr\\b" << "\\bsum\\b" << "\\bsuper\\b" << "\\btuple\\b" << "\\btype\\b" << "\\bunichr\\b"
                            << "\\bunicode\\b" << "\\bvars\\b" << "\\bxrange\\b" << "\\bzip\\b";

    foreach (const QString &pattern,  builtinFunctionPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = builtinFuncFormat;
        m_highlightingRules.append(rule);
    }

    rule.pattern = QRegExp("\\S*\\.");
    rule.format = objectFormat;
    m_highlightingRules.append(rule);

    QStringList exceptionPatterns;
    exceptionPatterns<< "\\bArithmeticError\\b" << "\\bAssertionError\\b" << "\\bAttributeError\\b"
                     << "\\bBaseException\\b" << "\\bDeprecationWarning\\b" << "\\bEnvironmentError\\b"
                     << "\\bEOFError\\b" << "\\bException\\b" << "\\bFloatingPointError\\b"
                     << "\\bFutureWarning\\b" << "\\bGeneratorExit\\b" << "\\bIOError\\b"
                     << "\\bImportError\\b" << "\\bImportWarning\\b" << "\\bIndexError\\b"
                     << "\\bKeyError\\b" << "\\bKeyboardInterrupt\\b" << "\\bLookupError\\b"
                     << "\\bMemoryError\\b" << "\\bNameError\\b" << "\\bNotImplementedError\\b"
                     << "\\bOSError\\b" << "\\bOverflowError\\b" << "\\bPendingDeprecationWarning\\b"
                     << "\\bReferenceError\\b" << "\\bRuntimeError\\b" << "\\bRuntimeWarning\\b"
                     << "\\bStandardError\\b" << "\\bStopIteration\\b" << "\\bSyntaxError\\b"
                     << "\\bSyntaxWarning\\b" << "\\bSystemError\\b" << "\\bSystemExit\\b"
                     << "\\bTypeError\\b" << "\\bUnboundLocalError\\b" << "\\bUserWarning\\b"
                     << "\\bUnicodeError\\b" << "\\bUnicodeWarning\\b" << "\\bUnicodeEncodeError\\b"
                     << "\\bUnicodeDecodeError\\b" << "\\bUnicodeTranslateError\\b" << "\\bValueError\\b"
                     << "\\bWarning\\b" << "\\bWindowsError\\b" << "\\bZeroDivisionError\\b";

    foreach (const QString &pattern,  exceptionPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = exceptionFormat;
        m_highlightingRules.append(rule);
    }

    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    m_highlightingRules.append(rule);

    rule.pattern= QRegExp("'.*'");
    rule.format = quotationFormat;
    m_highlightingRules.append(rule);

    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    m_highlightingRules.append(rule);

}

SageHighlighter::~SageHighlighter()
{
}

void SageHighlighter::highlightBlock(const QString& text)
{
    if(text.isEmpty())
        return;

    //Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    foreach (const HighlightingRule &rule,  m_highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index,  length,  rule.format);
            index = expression.indexIn(text,  index + length);
        }
    }

}
