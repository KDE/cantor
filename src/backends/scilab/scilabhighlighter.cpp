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
#include "result.h"
#include "textresult.h"
#include "session.h"

#include <QTextEdit>
#include <QDebug>

ScilabHighlighter::ScilabHighlighter(QObject* parent, Cantor::Session* session) : Cantor::DefaultHighlighter(parent), m_session(session)
{
    addKeywords(ScilabKeywords::instance()->keywords());
    addFunctions(ScilabKeywords::instance()->functions());
    addVariables(ScilabKeywords::instance()->variables());

    addRule(QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()")), functionFormat());

    addRule(QLatin1String("FIXME"), commentFormat());
    addRule(QLatin1String("TODO"), commentFormat());

    addRule(QRegularExpression(QStringLiteral("\"[^\"]*\"")), stringFormat());
    addRule(QRegularExpression(QStringLiteral("'[^']*'")), stringFormat());
    addRule(QRegularExpression(QStringLiteral("//[^\n]*")), commentFormat());

    commentStartExpression = QRegExp(QLatin1String("/\\*"));
    commentEndExpression = QRegExp(QLatin1String("\\*/"));
}

void ScilabHighlighter::highlightBlock(const QString& text)
{
    if (skipHighlighting(text)){
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

QString ScilabHighlighter::nonSeparatingCharacters() const
{
    return QLatin1String("[%]");
}
