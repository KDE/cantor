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
#include <kcolorscheme.h>
#include <kglobalsettings.h>
#include <kdebug.h>

using namespace Cantor;

class Cantor::DefaultHighlighterPrivate
{
  public:
    QTextEdit* parent;

    //Character formats to use for the highlighing
    QTextCharFormat functionFormat;
    QTextCharFormat variableFormat;
    QTextCharFormat objectFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat errorFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat matchingPairFormat;

    bool wasTextChanged;
    bool haveHighlightedPairs;
    // each two consecutive items build a pair
    QList<QChar> pairs;
};

DefaultHighlighter::DefaultHighlighter(QTextEdit* parent)
	: QSyntaxHighlighter(parent),
	d(new DefaultHighlighterPrivate)
{
    d->parent=parent;
    d->wasTextChanged=false;
    d->haveHighlightedPairs=false;

    addPair('(', ')');
    addPair('[', ']');
    addPair('{', '}');

    updateFormats();
    connect(KGlobalSettings::self(),  SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateFormats()));
    connect(parent, SIGNAL(cursorPositionChanged()), this, SLOT(positionChanged()));
    connect(parent->document(), SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
}

DefaultHighlighter::~ DefaultHighlighter()
{
}

void DefaultHighlighter::highlightBlock(const QString& text)
{
    if ( text.isEmpty())
        return;

    highlightPairs(text);
}

void DefaultHighlighter::addPair(const QChar& openSymbol, const QChar& closeSymbol)
{
    Q_ASSERT(!d->pairs.contains(openSymbol));
    Q_ASSERT(!d->pairs.contains(closeSymbol));
    d->pairs << openSymbol << closeSymbol;
}

void DefaultHighlighter::highlightPairs(const QString& text)
{
    d->haveHighlightedPairs = false;
    if (!d->parent->hasFocus()) {
        return;
    }

    int cursorPos = d->parent->textCursor().position() - currentBlock().position();

    if (cursorPos < 0)
        cursorPos = 0;

    if (cursorPos > text.size())
        return;

    highlightPairAtPos(cursorPos, text);
    if ( cursorPos > 0 ) {
      // Adjust cursorpos to allow for a symbol before the cursor position
      highlightPairAtPos(cursorPos - 1, text);
    }
}

void DefaultHighlighter::highlightPairAtPos(const int pos, const QString& text)
{
    int idx = d->pairs.indexOf(text[pos]);
    if ( idx == -1 ) {
        return;
    }

    QChar openSymbol;
    QChar closeSymbol;
    int inc; // which direction to search in

    if ( idx % 2 == 0 ) {
        // currently at openSymbol
        openSymbol = d->pairs[idx];
        closeSymbol = d->pairs[idx + 1];
        inc = +1;
    } else {
        // currently at closeSymbol
        openSymbol = d->pairs[idx - 1];
        closeSymbol = d->pairs[idx];
        inc = -1;
    }

    int level = 0;
    for (int i = pos; i >= 0 && i < text.size(); i += inc) {
        if (text[i] == closeSymbol)
            level--;
        else if (text[i] == openSymbol)
            level++;

        if (level == 0) {
            // Matched!
            setFormat(pos, 1, matchingPairFormat());
            setFormat(i, 1, matchingPairFormat());
            d->haveHighlightedPairs = true;
            return;
        }
    }
    setFormat(pos, 1, errorFormat());
}

DefaultHighlighter::BlockType DefaultHighlighter::currentBlockType()
{
    QTextBlock block=currentBlock();
    BlockType type=(BlockType) block.charFormat().intProperty(BlockTypeProperty);

    return type;
}

QTextCharFormat DefaultHighlighter::functionFormat() const
{
    return d->functionFormat;
}

QTextCharFormat DefaultHighlighter::variableFormat() const
{
    return d->variableFormat;
}

QTextCharFormat DefaultHighlighter::objectFormat() const
{
    return d->objectFormat;
}

QTextCharFormat DefaultHighlighter::keywordFormat() const
{
    return d->keywordFormat;
}

QTextCharFormat DefaultHighlighter::numberFormat() const
{
    return d->numberFormat;
}

QTextCharFormat DefaultHighlighter::operatorFormat() const
{
    return d->operatorFormat;
}

QTextCharFormat DefaultHighlighter::errorFormat() const
{
    return d->errorFormat;
}

QTextCharFormat DefaultHighlighter::commentFormat() const
{
    return d->commentFormat;
}

QTextCharFormat DefaultHighlighter::stringFormat() const
{
    return d->stringFormat;
}

QTextCharFormat DefaultHighlighter::matchingPairFormat() const
{
    return d->matchingPairFormat;
}

void DefaultHighlighter::updateFormats()
{
    //initialize char-formats
    KColorScheme scheme(QPalette::Active);

    d->functionFormat.setForeground(scheme.foreground(KColorScheme::LinkText));
    d->functionFormat.setFontWeight(QFont::DemiBold);

    d->variableFormat.setForeground(scheme.foreground(KColorScheme::ActiveText));

    d->objectFormat.setForeground(scheme.foreground(KColorScheme::NormalText));
    d->objectFormat.setFontWeight(QFont::Bold);

    d->keywordFormat.setForeground(scheme.foreground(KColorScheme::NeutralText));
    d->keywordFormat.setFontWeight(QFont::Bold);

    d->numberFormat.setForeground(scheme.foreground(KColorScheme::NeutralText));

    d->operatorFormat.setForeground(scheme.foreground(KColorScheme::NormalText));
    d->operatorFormat.setFontWeight(QFont::Bold);

    d->errorFormat.setForeground(scheme.foreground(KColorScheme::NormalText));
    d->errorFormat.setUnderlineColor(scheme.foreground(KColorScheme::NegativeText).color());
    d->errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    d->commentFormat.setForeground(scheme.foreground(KColorScheme::InactiveText));

    d->stringFormat.setForeground(scheme.foreground(KColorScheme::PositiveText));

    d->matchingPairFormat.setForeground(scheme.foreground(KColorScheme::NeutralText));
    d->matchingPairFormat.setBackground(scheme.background(KColorScheme::NeutralBackground));
}

void DefaultHighlighter::positionChanged()
{
    if ( d->wasTextChanged ) {
        d->wasTextChanged = false;
        return;
    }

    int pos = d->parent->textCursor().position();
    // don't do anything if we have not highlighted a pair and the current pos is not position-sensitive
    if ( !d->haveHighlightedPairs && !d->pairs.contains(d->parent->document()->characterAt(pos)) &&
         !( pos > 0 && d->pairs.contains(d->parent->document()->characterAt(pos - 1)) ) ) {
        return;
    }

    // either add the pair-highlighting or remove it
    rehighlightBlock(d->parent->textCursor().block());

    //rehighlighting causes a contentsChanged signal.
    //Make sure to mark that no text has changed since the last rehighlight
    d->wasTextChanged=false;
}

void DefaultHighlighter::contentsChanged()
{
    // editing text makes the cursor move, and triggers a re-highlight
    // so to prevent duplicate, useless highlighting eat the next positionChanged signal
    d->wasTextChanged = true;
}

#include  "defaulthighlighter.moc"
