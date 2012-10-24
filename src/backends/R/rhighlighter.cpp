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
    Copyright (C) 2010 Oleksiy Protas <elfy.ua@gmail.com>
 */

#include "rhighlighter.h"

#include <QTextEdit>
#include <kdebug.h>

const QStringList RHighlighter::keywords_list=QStringList()
    << "if" << "else" << "switch" << "while" << "for" << "repeat" << "function" << "in"
    << "next" << "break" << "TRUE" << "FALSE" << "NULL" << "NA" << "NA_integer_" << "NA_real_"
    << "NA_complex_" << "NA_character_" << "Inf" << "NaN";

const QStringList RHighlighter::operators_list=QStringList()
   << "(\\+|\\-|\\*|/|<-|->|<=|>=|={1,2}|\\!=|\\|{1,2}|&{1,2}|:{1,3}|\\^|@|\\$|~)((?!(\\+|\\-|\\*|/|<-|->|<=|>=|=|\\!=|\\||&|:|\\^|@|\\$|~))|$)"
   << "%[^%]*%"; // Taken in Kate highlighter

const QStringList RHighlighter::specials_list=QStringList()
    << "BUG" << "TODO" << "FIXME" << "NB" << "WARNING" << "ERROR";

RHighlighter::RHighlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    foreach (const QString& s, keywords_list)
        keywords.append(QRegExp("\\b"+s+"\\b"));
    foreach (const QString& s, operators_list)
        operators.append(QRegExp(s));
    foreach (const QString& s, specials_list)
        specials.append(QRegExp("\\b"+s+"\\b"));
}

RHighlighter::~RHighlighter()
{
}

void RHighlighter::refreshSyntaxRegExps()
{
    emit syntaxRegExps(variables,functions);
}

// FIXME: due to lack of lookbehinds in QRegExp here we use a flag showing if we need to shift the boundary of formating
// to make up for the accidently matched character
void RHighlighter::formatRule(const QRegExp &p, const QTextCharFormat &fmt, const QString& text,bool shift)
{
    int index = p.indexIn(text);
    while (index >= 0) {
        int length = p.matchedLength();
        setFormat(index+(shift?1:0),  length-(shift?1:0),  fmt);
       index = p.indexIn(text,  index + length);
    }
}

void RHighlighter::massFormat(const QVector<QRegExp> &p, const QTextCharFormat &fmt, const QString& text,bool shift)
{
    foreach (const QRegExp &rule, p)
        formatRule(rule,fmt,text,shift);
}


void RHighlighter::highlightBlock(const QString& text)
{
    if(text.isEmpty())
        return;

    //Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    //Let's mark every functionlike call as an error, then paint right ones in their respective format
    // TODO: find more elegant solution not involving double formatting
    formatRule(QRegExp("\\b[A-Za-z0-9_]+(?=\\()"),errorFormat(),text);

    //formatRule(QRegExp("[^A-Za-z_]-?([0-9]+)?(((e|i)?-?)|\\.)[0-9]*L?"),numberFormat(),text,true); // TODO: errorneous number formats, refine
    massFormat(keywords,keywordFormat(),text);
    massFormat(operators,operatorFormat(),text);
    massFormat(specials,commentFormat(),text); // FIXME must be distinc
    massFormat(functions,functionFormat(),text);
    massFormat(variables,variableFormat(),text);
    formatRule(QRegExp("\"[^\"]+\""),stringFormat(),text); // WARNING a bit redundant
}
