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

DefaultHighlighter::DefaultHighlighter(QTextEdit* parent)
	: QSyntaxHighlighter(parent),
	m_parent(parent)
{

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

    //BEGIN highlight matched brackets
    int cursorPos = m_parent->textCursor().position();
    if (cursorPos < 0)
        cursorPos = 0;

    // Adjust cursorpos to allow for a bracket before the cursor position
    if (cursorPos >= text.size())
        cursorPos = text.size() - 1;
    else if (cursorPos > 0 && (text[cursorPos-1] == '(' || text[cursorPos-1] == ')'))
		cursorPos--;

    bool haveOpen =  text[cursorPos] == '(';
    bool haveClose = text[cursorPos] == ')';

    if ((haveOpen || haveClose) && m_parent->hasFocus())
    {
        // Search for the other bracket

        int inc = haveOpen ? 1 : -1; // which direction to search in

        int level = 0;
        for (int i = cursorPos; i >= 0 && i < text.size(); i += inc) {
            if (text[i] == ')')
                level--;
            else if (text[i] == '(')
                level++;

            if (level == 0) {
                // Matched!
                setFormat(cursorPos, 1, matchedParenthesis);
                setFormat(i, 1, matchedParenthesis);
                break;
            }
        }
    }
    //END highlight matched brackets
}

