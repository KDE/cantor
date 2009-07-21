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
    Copyright (C) 2006 David Saxton <david@bluehaze.org>
*/

#include "defaulthighlighter.h"

#include <QtCore/QLocale>
#include <QTextEdit>

using namespace MathematiK;

class MathematiK::DefaultHighlighterPrivate
{
  public:
    QTextEdit* parent;
};

DefaultHighlighter::DefaultHighlighter(QTextEdit* parent)
	: QSyntaxHighlighter(parent),
	d(new DefaultHighlighterPrivate)
{
    d->parent=parent;
}


DefaultHighlighter::~ DefaultHighlighter()
{
}


void DefaultHighlighter::highlightBlock(const QString& text)
{
    // Good color defaults borrowed from Abakus - thanks! :)

    if (text.isEmpty())
        return;

    QTextCharFormat number;
    number.setForeground(QColor(0, 0, 127));

    QTextCharFormat function;
    function.setForeground(QColor(85, 0, 0));

    QTextCharFormat variable;
    variable.setForeground(QColor(0, 85, 0));

    QTextCharFormat matchedParenthesis;
    matchedParenthesis.setBackground(QColor(255, 255, 183));

    QTextCharFormat other;

    //highlight brackets
    matchBrackets('(', ')', matchedParenthesis, text);
    matchBrackets('[', ']', matchedParenthesis, text);
    matchBrackets('{', '}', matchedParenthesis, text);
}

void DefaultHighlighter::matchBrackets(QChar openSymbol, QChar closeSymbol, QTextCharFormat& format, const QString& text)
{
    //BEGIN highlight matched brackets
    int cursorPos = d->parent->textCursor().position();
    if (cursorPos < 0)
        cursorPos = 0;

    // Adjust cursorpos to allow for a bracket before the cursor position
    if (cursorPos >= text.size())
        cursorPos = text.size() - 1;
    else if (cursorPos > 0 && (text[cursorPos-1] == openSymbol || text[cursorPos-1] == closeSymbol))
		cursorPos--;

    bool haveOpen =  text[cursorPos] == openSymbol;
    bool haveClose = text[cursorPos] == closeSymbol;

    if ((haveOpen || haveClose) && d->parent->hasFocus())
    {
        // Search for the other bracket

        int inc = haveOpen ? 1 : -1; // which direction to search in

        int level = 0;
        for (int i = cursorPos; i >= 0 && i < text.size(); i += inc) {
            if (text[i] == closeSymbol)
                level--;
            else if (text[i] == openSymbol)
                level++;

            if (level == 0) {
                // Matched!
                setFormat(cursorPos, 1, format);
                setFormat(i, 1, format);
                break;
            }
        }
    }
    //END highlight matched brackets
}

