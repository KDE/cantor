/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/

#include "juliahighlighter.h"
#include "juliakeywords.h"
#include "juliasession.h"

#include <climits>
#include <QTextEdit>
#include <QDebug>

JuliaHighlighter::JuliaHighlighter(QObject *parent, JuliaSession* session)
    : Cantor::DefaultHighlighter(parent, session)
{
    addKeywords(JuliaKeywords::instance()->keywords());
    addVariables(JuliaKeywords::instance()->variables());
    addFunctions(JuliaKeywords::instance()->functions());
}

void JuliaHighlighter::highlightBlock(const QString &text)
{
    if (skipHighlighting(text)) {
        return;
    }

    // Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    // Now we are about to make correct strings and comments highlighting
    //
    // Main idea: as soon as string starts comment or anything else cant start
    // until current string ends. The same with comment, except '#' comment
    // that ends by newline
    //
    // To pass information to next block, we are using next states
    const int IN_MULTILINE_COMMENT = 1;
    const int IN_CHARACTER = 2;
    const int IN_SINGLE_QUOTE_STRING = 4;
    const int IN_TRIPLE_QUOTE_STRING = 8;

    // Markers of scopes start, ends
    static const QRegularExpression multiLineCommentStart(QStringLiteral("#="));
    static const QRegularExpression multiLineCommentEnd(QStringLiteral("=#"));
    static const QRegularExpression characterStartEnd(QStringLiteral("'"));
    static const QRegularExpression singleQuoteStringStartEnd(QStringLiteral("\""));
    static const QRegularExpression tripleQuoteStringStartEnd(QStringLiteral("\"\"\""));
    static const QRegularExpression singleLineCommentStart(QStringLiteral("#(?!=)"));

    // Get current state
    int state = previousBlockState();
    if (state == -1) {
        state = 0;
    }

    // This 4 arrays establish matching between state, start marker, end marker
    // and format to apply
    QList<int> flags = {
        IN_TRIPLE_QUOTE_STRING,
        IN_SINGLE_QUOTE_STRING,
        IN_CHARACTER,
        IN_MULTILINE_COMMENT
    };
    const QVector<QRegularExpression> regexps_starts = {
        tripleQuoteStringStartEnd,
        singleQuoteStringStartEnd,
        characterStartEnd,
        multiLineCommentStart
    };
    const QVector<QRegularExpression> regexps_ends = {
        tripleQuoteStringStartEnd,
        singleQuoteStringStartEnd,
        characterStartEnd,
        multiLineCommentEnd
    };
    QList<QTextCharFormat> formats = {
        stringFormat(),
        stringFormat(),
        stringFormat(),
        commentFormat()
    };

    int pos = 0; // current position in block
    while (pos < text.length()) {
        // Trying to close current environments
        bool triggered = false;
        for (int i = 0; i < flags.size() && !triggered; i++) {
            int flag = flags[i];
            QTextCharFormat &format = formats[i];
            if (state & flag) { // Found current state
                // find where end marker is
                const QRegularExpressionMatch match = regexps_ends.at(i).match(text, pos);
                const int new_pos = match.capturedStart(0);
                int length;
                if (new_pos == -1) {
                    // not in this block, highlight till the end
                    length = text.length() - pos;
                } else {
                    // highlight untill the marker and modify state
                    length = new_pos - pos + match.capturedLength(0);
                    state -= flag;
                }
                // Apply format to the found area
                setFormat(pos, length, format);
                pos = pos + length;
                triggered = true;
            }
        }
        if (triggered) { // We have done something move to next iteration
            continue;
        }

        // Now we should found the scope that start the closest to current
        // position
        QRegularExpressionMatch match; // closest marker
        int minPos = INT_MAX; // closest pos
        int minIdx = -1; // closest scope index
        for (int i = 0; i < regexps_starts.size(); i++) {
            match = regexps_starts.at(i).match(text, pos);
            const int newPos = match.capturedStart(0);
            if (newPos != -1) {
                minPos = qMin(minPos, newPos);
                minIdx = i;
            }
        }

        // Check where single line comment starts
        const int singleLineCommentStartPos = text.indexOf(singleLineCommentStart, pos);

        if (singleLineCommentStartPos != -1
                && singleLineCommentStartPos < minPos) {
            // single line comment starts earlier
            setFormat(singleLineCommentStartPos, text.length() - singleLineCommentStartPos, commentFormat());
            break;
        } else if (match.hasMatch()) {
            // We are going to another scope
            state += flags[minIdx];
            pos = minPos +  match.capturedLength(0);
            setFormat(minPos, match.capturedLength(0), formats[minIdx]);
        } else { // There is nothing to highlight
            break;
        }
    }

    setCurrentBlockState(state);
}

QString JuliaHighlighter::nonSeparatingCharacters() const
{
    return QLatin1String("[\\w¡-ﻼ!]");
}
