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

#ifndef DEFAULTHIGHLIGHTER_H
#define DEFAULTHIGHLIGHTER_H

#include "cantor_export.h"

#include <QtGui/QSyntaxHighlighter>

namespace Cantor
{
class DefaultHighlighterPrivate;

class CANTOR_EXPORT DefaultHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT
  public:
    enum BlockType {UnknownBlock = 0, ErrorBlock = 1, ResultBlock = 2, CommandBlock = 3 };
    enum { BlockTypeProperty = QTextFormat::UserProperty +25 };
    DefaultHighlighter(QTextEdit* parent);
    ~DefaultHighlighter();

  protected:
    virtual void highlightBlock(const QString& text);

    BlockType currentBlockType();

    QTextCharFormat functionFormat() const;
    QTextCharFormat variableFormat() const;
    QTextCharFormat objectFormat() const;
    QTextCharFormat keywordFormat() const;
    QTextCharFormat numberFormat() const;
    QTextCharFormat operatorFormat() const;
    QTextCharFormat errorFormat() const;
    QTextCharFormat commentFormat() const;
    QTextCharFormat stringFormat() const;
    QTextCharFormat matchingPairFormat() const;

    /// Call this to add a pair of symbols for highlighting
    /// Default implementation already adds (), {} and [].
    void addPair(const QChar& openSymbol, const QChar& closeSymbol);

    /// highlight added pairs
    void highlightPairs(const QString& text);

  private slots:
    void positionChanged();
    void updateFormats();

  private:
    DefaultHighlighterPrivate* d;
};

}

#endif
