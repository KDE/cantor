/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#include "pythonhighlighter.h"
#include "pythonkeywords.h"
#include "pythonsession.h"

PythonHighlighter::PythonHighlighter(QObject* parent, PythonSession* session) : Cantor::DefaultHighlighter(parent, session)
{
    addRule(QRegularExpression(QStringLiteral("\\b\\w+(?=\\()")), functionFormat());

    //Code highlighting the different keywords
    addKeywords(PythonKeywords::instance()->keywords());
    addFunctions(PythonKeywords::instance()->functions());
    addVariables(PythonKeywords::instance()->variables());
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    if (skipHighlighting(text))
        return;

    // Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    const int IN_MULTILINE_COMMENT = 1;
    const int IN_SMALL_QUOTE_STRING = 2;
    const int IN_SINGLE_QUOTE_STRING = 4;
    const int IN_TRIPLE_QUOTE_STRING = 8;

    static const QRegularExpression multiLineCommentStartEnd(QStringLiteral("'''"));
    static const QRegularExpression smallQuoteStartEnd(QStringLiteral("'"));
    static const QRegularExpression singleQuoteStringStartEnd(QStringLiteral("\""));
    static const QRegularExpression tripleQuoteStringStartEnd(QStringLiteral("\"\"\""));
    static const QRegularExpression singleLineCommentStart(QStringLiteral("#"));

    int state = previousBlockState();
    if (state == -1) {
        state = 0;
    }

    QList<int> flags = {
        IN_TRIPLE_QUOTE_STRING,
        IN_SINGLE_QUOTE_STRING,
        IN_SMALL_QUOTE_STRING,
        IN_MULTILINE_COMMENT
    };
    const QVector<QRegularExpression> regexps = {
        tripleQuoteStringStartEnd,
        singleQuoteStringStartEnd,
        smallQuoteStartEnd,
        multiLineCommentStartEnd
    };
    QList<QTextCharFormat> formats = {
        stringFormat(),
        stringFormat(),
        stringFormat(),
        commentFormat()
    };

    int pos = 0;
    while (pos < text.length()) {
        // Trying to close current environments
        bool triggered = false;
        for (int i = 0; i < flags.size() && !triggered; i++) {
            int flag = flags[i];
            QTextCharFormat &format = formats[i];
            if (state & flag) {
                const QRegularExpressionMatch match = regexps.at(i).match(text, pos);
                int length;
                if (!match.hasMatch()) {
                    length = text.length() - pos;
                } else { // found a match
                    length = match.capturedStart(0) - pos + match.capturedLength(0);
                    state -= flag;
                }
                setFormat(pos, length, format);
                pos = pos + length;
                triggered = true;
            }
        }
        if (triggered) {
            continue;
        }

        QRegularExpressionMatch minMatch;
        int minPos = INT_MAX;
        int minIdx = -1;
        for (int i = 0; i < regexps.size(); i++) {
            const QRegularExpressionMatch match = regexps.at(i).match(text, pos);
            if (match.hasMatch()) {
                minPos = qMin(minPos, match.capturedStart(0));
                minIdx = i;
                minMatch = match;
            }
        }

        const int singleLineCommentStartPos = text.indexOf(singleLineCommentStart, pos);

        if (singleLineCommentStartPos != -1
            && singleLineCommentStartPos < minPos) {
            setFormat(singleLineCommentStartPos, text.length() - singleLineCommentStartPos, commentFormat());
        break;
            } else if (minMatch.hasMatch()) {
                state += flags[minIdx];
                pos = minPos + minMatch.capturedLength(0);
                setFormat(minPos, minMatch.capturedLength(0), formats[minIdx]);
            } else {
                break;
            }
    }

    setCurrentBlockState(state);
}
