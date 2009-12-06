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

#include "maximahighlighter.h"
#include "maximakeywords.h"

#include <QTextEdit>
#include <kdebug.h>

MaximaHighlighter::MaximaHighlighter(QTextEdit* edit) : Cantor::DefaultHighlighter(edit)
{
    HighlightingRule rule;

    //Setup the highlighting rules
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat();
    m_highlightingRules.append(rule);

    //Code highlighting the different keywords
    foreach (const QString &pattern, MaximaKeywords::keywords())
    {
        rule.pattern = QRegExp("\\b"+QRegExp::escape(pattern)+"\\b");
        rule.format = keywordFormat();
        m_highlightingRules.append(rule);
    }


    QStringList specialCommentPatterns;
    specialCommentPatterns
        <<"\\bFIXME\\b" <<"\\bTODO\\b" ;

    foreach (const QString &pattern, specialCommentPatterns )
    {
        rule.pattern = QRegExp(pattern);
        rule.format = commentFormat();
        m_highlightingRules.append(rule);
    }


    foreach (const QString &pattern, MaximaKeywords::functions() )
    {
        rule.pattern = QRegExp("\\b"+QRegExp::escape(pattern)+"\\b");
        rule.format = functionFormat();
        m_highlightingRules.append(rule);
    }


    foreach (const QString &pattern, MaximaKeywords::variables() )
    {
        rule.pattern = QRegExp("\\b"+QRegExp::escape(pattern)+"\\b");
        rule.format = variableFormat();
        m_highlightingRules.append(rule);
    }


    rule.pattern = QRegExp("\".*\"");
    rule.format = stringFormat();
    m_highlightingRules.append(rule);

    rule.pattern= QRegExp("'.*'");
    rule.format = stringFormat();
    m_highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

MaximaHighlighter::~MaximaHighlighter()
{
}

void MaximaHighlighter::highlightBlock(const QString& text)
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

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {

        int endIndex = commentEndExpression.indexIn(text,  startIndex);
        int commentLength;
        if (endIndex == -1) {

            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                + commentEndExpression.matchedLength();
        }
        setFormat(startIndex,  commentLength,  commentFormat());
        startIndex = commentStartExpression.indexIn(text,  startIndex + commentLength);
    }
}
