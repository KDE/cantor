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

#include "scilabhighlighter.h"
#include "scilabkeywords.h"

#include <QTextEdit>
#include <QDebug>

ScilabHighlighter::ScilabHighlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    qDebug() << "ScilabHighlighter construtor";
    addRule(QRegExp(QLatin1String("\\b[A-Za-z0-9_]+(?=\\()")), functionFormat());

    //Code highlighting the different keywords
    addKeywords(ScilabKeywords::instance()->keywords());

    addRule(QLatin1String("FIXME"), commentFormat());
    addRule(QLatin1String("TODO"), commentFormat());

    addFunctions(ScilabKeywords::instance()->functions());
    addVariables(ScilabKeywords::instance()->variables());

    addRule(QRegExp(QLatin1String("\"[^\"]*\"")), stringFormat());
    addRule(QRegExp(QLatin1String("'[^']*'")), stringFormat());
    addRule(QRegExp(QLatin1String("//[^\n]*")), commentFormat());

    commentStartExpression = QRegExp(QLatin1String("/\\*"));
    commentEndExpression = QRegExp(QLatin1String("\\*/"));
}

ScilabHighlighter::~ScilabHighlighter()
{
}

void ScilabHighlighter::highlightBlock(const QString& text)
{
    qDebug() << "ScilabHighlighter::highlightBlock";
    qDebug() << "text: " << text;

    if (skipHighlighting(text)){
        qDebug() << "skipHighlighting(" << text << " ) " << "== true";
        return;
    }

    //Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0){

        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1){

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

void ScilabHighlighter::addVariableHighlight()
{
    addVariables(ScilabKeywords::instance()->variables());
    rehighlight();
}

void ScilabHighlighter::updateHighlight()
{
    qDebug();

    addVariables(ScilabKeywords::instance()->variables());
    addKeywords(ScilabKeywords::instance()->keywords());
    addFunctions(ScilabKeywords::instance()->functions());

    rehighlight();
}

QString ScilabHighlighter::nonSeparatingCharacters() const
{
    qDebug() << "ScilabHighlighter::nonSeparatingCharacters() function";
    return QLatin1String("[%]");
}
