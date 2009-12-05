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

#ifndef _MAXIMAHIGHLIGHTER_H
#define _MAXIMAHIGHLIGHTER_H

#include "defaulthighlighter.h"

class MaximaHighlighter : public Cantor::DefaultHighlighter
{
  public:
    MaximaHighlighter( QTextEdit* edit);
    ~MaximaHighlighter();

  protected:
    void highlightBlock(const QString &text);

  private:
     struct HighlightingRule
     {
	     QRegExp pattern;
	     QTextCharFormat format;
     };

     QVector<HighlightingRule> m_highlightingRules;

     QRegExp commentStartExpression;
     QRegExp commentEndExpression;
};

#endif /* _MAXIMAHIGHLIGHTER_H */
