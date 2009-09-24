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
 */

#ifndef _SAGEHIGHLIGHTER_H
#define _SAGEHIGHLIGHTER_H

#include "defaulthighlighter.h"


/*
  this is basically a syntax highlighter for the 
  Python programming Language, as Sage is based on
  it
*/
class SageHighlighter : public Cantor::DefaultHighlighter
{
  public:
    SageHighlighter( QTextEdit* edit);
    ~SageHighlighter();

  protected:
    void highlightBlock(const QString &text);

  private:
     struct HighlightingRule
     {
	     QRegExp pattern;
	     QTextCharFormat format;
     };

     QVector<HighlightingRule> m_highlightingRules;

     //Different formats, used for highlighting
     QTextCharFormat keywordFormat;
     QTextCharFormat builtinFuncFormat;
     QTextCharFormat objectFormat;
     QTextCharFormat exceptionFormat;
     QTextCharFormat singleLineCommentFormat;
     QTextCharFormat quotationFormat;
     QTextCharFormat functionFormat;
};

#endif /* _SAGEHIGHLIGHTER_H */
