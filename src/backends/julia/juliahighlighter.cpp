/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */

#include "juliahighlighter.h"
#include "juliakeywords.h"

#include <climits>
#include <QTextEdit>

JuliaHighlighter::JuliaHighlighter(QObject *parent)
    : Cantor::DefaultHighlighter(parent)
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

    // Now we are about to make corrent strings and comments highlighting
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
    QRegExp multiLineCommentStart(QLatin1String("#="));
    QRegExp multiLineCommentEnd(QLatin1String("=#"));
    QRegExp characterStartEnd(QLatin1String("'"));
    QRegExp singleQuoteStringStartEnd(QLatin1String("\""));
    QRegExp tripleQuoteStringStartEnd(QLatin1String("\"\"\""));
    QRegExp singleLineCommentStart(QLatin1String("#(?!=)"));

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
    QList<QRegExp> regexps_starts = {
        tripleQuoteStringStartEnd,
        singleQuoteStringStartEnd,
        characterStartEnd,
        multiLineCommentStart
    };
    QList<QRegExp> regexps_ends = {
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
            QRegExp &regexp = regexps_ends[i];
            QTextCharFormat &format = formats[i];
            if (state & flag) { // Found current state
                // find where end marker is
                int new_pos = regexp.indexIn(text, pos);
                int length;
                if (new_pos == -1) {
                    // not in this block, highlight till the end
                    length = text.length() - pos;
                } else {
                    // highlight untill the marker and modify state
                    length = new_pos - pos + regexp.matchedLength();
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
        QRegExp *minRegexp = nullptr; // closest marker
        int minPos = INT_MAX; // closest pos
        int minIdx = -1; // closest scope index
        for (int i = 0; i < regexps_starts.size(); i++) {
            QRegExp &regexp = regexps_starts[i];
            int newPos = regexp.indexIn(text, pos);
            if (newPos != -1) {
                minPos = qMin(minPos, newPos);
                minRegexp = &regexp;
                minIdx = i;
            }
        }

        // Check where single line comment starts
        int singleLineCommentStartPos =
            singleLineCommentStart.indexIn(text, pos);

        if (singleLineCommentStartPos != -1
                && singleLineCommentStartPos < minPos) {
            // single line comment starts earlier
            setFormat(pos, text.length() - pos, commentFormat());
            break;
        } else if (minRegexp) {
            // We are going to another scope
            state += flags[minIdx];
            pos = minPos +  minRegexp->matchedLength();
            setFormat(minPos, minRegexp->matchedLength(), formats[minIdx]);
        } else { // There is nothing to highlight
            break;
        }
    }

    setCurrentBlockState(state);
}

void JuliaHighlighter::updateHighlight()
{
    // Remove rules for outdated variables and functions
    for (const auto &var : JuliaKeywords::instance()->removedVariables()) {
        removeRule(var);
    }
    for (const auto &func : JuliaKeywords::instance()->removedFunctions()) {
        removeRule(func);
    }

    // Add actual variables and function
    addVariables(JuliaKeywords::instance()->variables());
    addFunctions(JuliaKeywords::instance()->functions());
    rehighlight();
}

QString JuliaHighlighter::nonSeparatingCharacters() const
{
    return QLatin1String("[\\w\\u00A1-\\uFFFF!]");
}
