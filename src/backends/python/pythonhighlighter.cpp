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
#include <QDebug>

PythonHighlighter::PythonHighlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    qDebug() << "PythonHighlighter construtor";
    addRule(QRegExp(QLatin1String("\\b\\w+(?=\\()")), functionFormat());

    //Code highlighting the different keywords
    addKeywords(PythonKeywords::instance()->keywords());
    addFunctions(PythonKeywords::instance()->functions());
    addVariables(PythonKeywords::instance()->variables());
}

PythonHighlighter::~PythonHighlighter()
{
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    if (skipHighlighting(text)) {
        return;
    }

    // Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    const int IN_MULTILINE_COMMENT = 1;
    const int IN_SMALL_QUOTE_STRING = 2;
    const int IN_SINGLE_QUOTE_STRING = 4;
    const int IN_TRIPLE_QUOTE_STRING = 8;

    QRegExp multiLineCommentStartEnd(QLatin1String("'''"));
    QRegExp smallQuoteStartEnd(QLatin1String("'"));
    QRegExp singleQuoteStringStartEnd(QLatin1String("\""));
    QRegExp tripleQuoteStringStartEnd(QLatin1String("\"\"\""));
    QRegExp singleLineCommentStart(QLatin1String("#"));

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
    QList<QRegExp> regexps = {
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
            QRegExp &regexp = regexps[i];
            QTextCharFormat &format = formats[i];
            if (state & flag) {
                int new_pos = regexp.indexIn(text, pos);
                int length;
                if (new_pos == -1) {
                    length = text.length() - pos;
                } else {
                    length = new_pos - pos + regexp.matchedLength();
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

        QRegExp *minRegexp = nullptr;
        int minPos = INT_MAX;
        int minIdx = -1;
        for (int i = 0; i < regexps.size(); i++) {
            QRegExp &regexp = regexps[i];
            int newPos = regexp.indexIn(text, pos);
            if (newPos != -1) {
                minPos = qMin(minPos, newPos);
                minRegexp = &regexp;
                minIdx = i;
            }
        }

        int singleLineCommentStartPos =
        singleLineCommentStart.indexIn(text, pos);

        if (singleLineCommentStartPos != -1
            && singleLineCommentStartPos < minPos) {
            setFormat(pos, text.length() - pos, commentFormat());
        break;
            } else if (minRegexp) {
                state += flags[minIdx];
                pos = minPos +  minRegexp->matchedLength();
                setFormat(minPos, minRegexp->matchedLength(), formats[minIdx]);
            } else {
                break;
            }
    }

    setCurrentBlockState(state);
}

void PythonHighlighter::updateHighlight()
{

    addVariables(PythonKeywords::instance()->variables());
    rehighlight();
}
