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

#include "pythonhighlighter.h"
#include "pythonkeywords.h"

#include <QTextEdit>
#include <kdebug.h>

PythonHighlighter::PythonHighlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    kDebug() << "PythonHighlighter construtor";
    addRule(QRegExp("\\b[A-Za-z0-9_]+(?=\\()"), functionFormat());

    //Code highlighting the different keywords
    addKeywords(PythonKeywords::instance()->keywords());

    addRule("FIXME", commentFormat());
    addRule("TODO", commentFormat());

    addFunctions(PythonKeywords::instance()->functions());
    addVariables(PythonKeywords::instance()->variables());

    addRule(QRegExp("\".*\""), stringFormat());
    addRule(QRegExp("'.*'"), stringFormat());
    addRule(QRegExp("#[^\n]*"), commentFormat());

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

PythonHighlighter::~PythonHighlighter()
{
}

void PythonHighlighter::highlightBlock(const QString& text)
{
    kDebug() << "PythonHighlighter::highlightBlock";
    kDebug() << "text: " << text;

    if (skipHighlighting(text)){
        kDebug() << "skipHighlighting(" << text << " ) " << "== true";
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

QString PythonHighlighter::nonSeparatingCharacters() const
{
    kDebug() << "PythonHighlighter::nonSeparatingCharacters() function";
    return "[%]";
}
