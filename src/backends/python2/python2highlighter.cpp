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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#include "python2highlighter.h"
#include "python2keywords.h"

#include <QTextEdit>
#include <QDebug>

Python2Highlighter::Python2Highlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    qDebug() << "Python2Highlighter construtor";
    addRule(QRegExp(QLatin1String("\\b[A-Za-z0-9_]+(?=\\()")), functionFormat());

    //Code highlighting the different keywords
    addKeywords(Python2Keywords::instance()->keywords());

    addRule(QLatin1String("FIXME"), commentFormat());
    addRule(QLatin1String("TODO"), commentFormat());

    addFunctions(Python2Keywords::instance()->functions());
    addVariables(Python2Keywords::instance()->variables());

    addRule(QRegExp(QLatin1String("\".*\"")), stringFormat());
    addRule(QRegExp(QLatin1String("'.*'")), stringFormat());
    addRule(QRegExp(QLatin1String("#[^\n]*")), commentFormat());

    commentStartExpression = QRegExp(QLatin1String("'''[^\n]*"));
    commentEndExpression = QRegExp(QLatin1String("'''"));
}

Python2Highlighter::~Python2Highlighter()
{
}

void Python2Highlighter::highlightBlock(const QString& text)
{
    qDebug() << "Python2Highlighter::highlightBlock";
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

    while (startIndex >= 0) {

        int endIndex = commentEndExpression.indexIn(text, startIndex);
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

void Python2Highlighter::updateHighlight()
{
    qDebug();

    addVariables(Python2Keywords::instance()->variables());
    rehighlight();
}

QString Python2Highlighter::nonSeparatingCharacters() const
{
    qDebug() << "Python2Highlighter::nonSeparatingCharacters() function";
    return QLatin1String("[%]");
}
