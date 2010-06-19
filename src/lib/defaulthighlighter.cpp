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
#include <QStack>

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

    int lastBlockNumber;
    int lastPosition;
    // each two consecutive items build a pair
    QList<QChar> pairs;
};

DefaultHighlighter::DefaultHighlighter(QTextEdit* parent)
	: QSyntaxHighlighter(parent),
	d(new DefaultHighlighterPrivate)
{
    d->parent=parent;
    d->lastBlockNumber=-1;
    d->lastPosition=-1;

    addPair('(', ')');
    addPair('[', ']');
    addPair('{', '}');

    updateFormats();
    connect(KGlobalSettings::self(),  SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateFormats()));
    connect(parent, SIGNAL(cursorPositionChanged()), this, SLOT(positionChanged()));
}

DefaultHighlighter::~ DefaultHighlighter()
{
    delete d;
}

bool DefaultHighlighter::skipHighlighting(const QString& text)
{
    return (text.isEmpty() || currentBlockType() == NoHighlightBlock);
}

void DefaultHighlighter::highlightBlock(const QString& text)
{
    if (skipHighlighting(text))
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
    const QTextCursor& cursor = d->parent->textCursor();
    int cursorPos = -1;
    if ( cursor.blockNumber() == currentBlock().blockNumber() ) {
        cursorPos = cursor.position() - currentBlock().position();
        // when text changes, this will be called before the positionChanged signal
        // gets emitted. Hence update the position so we don't highlight twice
        d->lastPosition = cursor.position();
    }

    // positions of opened pairs
    // key: same index as the opener has in d->pairs
    // value: position in text where it was opened
    QHash<int, QStack<int>* > opened;
    QHash<int, QStack<int>* >::iterator it;

    ///TODO: use setCurrentBlockUserData to keep track of matched pairs
    ///      of course, keep track of changes and update the cache properly
    for ( int i = 0; i < text.size(); ++i ) {
        int idx = d->pairs.indexOf(text[i]);
        if ( idx != -1 ) {
            if ( idx % 2 == 0 ) {
                // opener of a pair
                it = opened.find(idx);
                if ( it == opened.end() ) {
                    it = opened.insert(idx, new QStack<int>());
                }
                (*it)->push(i);
            } else {
                // closer of a pair, find opener
                it = opened.find(idx - 1);
                if ( it == opened.end() || (*it)->isEmpty() ) {
                    // unmatched
                    setFormat(i, 1, errorFormat());
                } else {
                    // matched
                    int lastPos = (*it)->pop();
                    // check if we have to highlight the matched pair
                    // at the current cursor position
                    if ( cursorPos != -1 &&
                        ( lastPos == cursorPos || lastPos == cursorPos - 1 ||
                            i == cursorPos || i == cursorPos - 1 ) )
                    {
                        // yep, we want it highlighted
                        setFormat(lastPos, 1, matchingPairFormat());
                        setFormat(i, 1, matchingPairFormat());
                    }
                }
            }
        }
    }

    // handled unterminated pairs
    foreach ( QStack<int>* positions, opened ) {
        while ( !positions->isEmpty() ) {
            setFormat(positions->pop(), 1, errorFormat());
        }
    }
    qDeleteAll(opened.values());
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
    const QTextCursor& cursor = d->parent->textCursor();

    if ( cursor.blockNumber() != d->lastBlockNumber ) {
        // remove highlight from last focused block
        kDebug() << "cleaning up last block";
        rehighlightBlock(d->parent->document()->findBlockByNumber(d->lastBlockNumber));
    }

    d->lastBlockNumber = cursor.blockNumber();

    if ( d->lastPosition == cursor.position() ) {
        return;
    }

    kDebug() << "position changed, rehighlight block";

    rehighlightBlock(cursor.block());
    d->lastPosition = cursor.position();
}

#include  "defaulthighlighter.moc"
