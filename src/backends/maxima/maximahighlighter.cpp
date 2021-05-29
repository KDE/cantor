/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "maximahighlighter.h"
#include "maximakeywords.h"
#include "maximasession.h"
#include "maximavariablemodel.h"

MaximaHighlighter::MaximaHighlighter(QObject* parent, MaximaSession* session)
    : Cantor::DefaultHighlighter(parent, session)
{
    //addRule(QRegExp("\\b[A-Za-z0-9_]+(?=\\()"), functionFormat());

    //Code highlighting the different keywords
    addKeywords(MaximaKeywords::instance()->keywords());

    addRule(QLatin1String("FIXME"), commentFormat());
    addRule(QLatin1String("TODO"), commentFormat());

    addFunctions(MaximaKeywords::instance()->functions());
    addVariables(MaximaKeywords::instance()->variables());

    //addRule(QRegExp("\".*\""), stringFormat());
    //addRule(QRegExp("'.*'"), stringFormat());

    commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
    commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}

void MaximaHighlighter::highlightBlock(const QString& text)
{
    if (skipHighlighting(text))
        return;

    //Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    setCurrentBlockState(-1);

    int commentLevel = 0;
    bool inString = false;
    int startIndex = -1;

    if (previousBlockState() > 0) {
        commentLevel = previousBlockState();
        startIndex = 0;
    } else if (previousBlockState() < -1) {
        inString = true;
        startIndex = 0;
    }

    for (int i = 0; i < text.size(); ++i) {
        if (text[i] == QLatin1Char('\\')) {
            ++i; // skip the next character
        } else if (text[i] == QLatin1Char('"') && commentLevel == 0) {
            if (!inString)
                startIndex = i;
            else
                setFormat(startIndex, i - startIndex + 1, stringFormat());
            inString = !inString;
        } else if (text.mid(i,2) == QLatin1String("/*") && !inString) {
            if (commentLevel == 0)
                startIndex = i;
            ++commentLevel;
            ++i;
        } else if (text.mid(i,2) == QLatin1String("*/") && !inString) {
            if (commentLevel == 0) {
                setFormat(i, 2, errorFormat());
                // undo the --commentLevel below, so we stay at 0
                ++commentLevel;
            } else if (commentLevel == 1) {
                setFormat(startIndex, i - startIndex + 2, commentFormat());
            }
            ++i;
            --commentLevel;
        }
    }

    if (inString) {
        setCurrentBlockState(-2);
        setFormat(startIndex, text.size() - startIndex, stringFormat());
    } else if (commentLevel > 0) {
        setCurrentBlockState(commentLevel);
        setFormat(startIndex, text.size() - startIndex, commentFormat());
    }
}

QString MaximaHighlighter::nonSeparatingCharacters() const
{
    return QLatin1String("%");
}
