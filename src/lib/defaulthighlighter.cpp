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
#include <QTextDocument>
#include <QTextCursor>
#include <QGraphicsTextItem>
#include <kcolorscheme.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <QStack>

using namespace Cantor;

struct HighlightingRule
{
    QRegExp regExp;
    QTextCharFormat format;
};

bool operator==(const HighlightingRule& rule1, const HighlightingRule& rule2)
{
    return rule1.regExp == rule2.regExp;
}

class Cantor::DefaultHighlighterPrivate
{
  public:
    QTextCursor cursor;

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

    QList<HighlightingRule> regExpRules;
    QHash<QString, QTextCharFormat> wordRules;
};

DefaultHighlighter::DefaultHighlighter(QObject* parent)
	: QSyntaxHighlighter(parent),
	d(new DefaultHighlighterPrivate)
{
    d->cursor = QTextCursor();
    d->lastBlockNumber=-1;
    d->lastPosition=-1;

    addPair('(', ')');
    addPair('[', ']');
    addPair('{', '}');

    updateFormats();
    connect(KGlobalSettings::self(),  SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateFormats()));
}

DefaultHighlighter::~DefaultHighlighter()
{
    delete d;
}

void DefaultHighlighter::setTextItem(QGraphicsTextItem* item)
{
    d->cursor = item->textCursor();
    setDocument(item->document());
    // make sure every item is connected only once
    item->disconnect(this, SLOT(positionChanged(QTextCursor)));
    // QGraphicsTextItem has no signal cursorPositionChanged, but item really
    // is a WorksheetTextItem
    connect(item, SIGNAL(cursorPositionChanged(QTextCursor)),
	    this, SLOT(positionChanged(QTextCursor)));

    d->lastBlockNumber = -1;
    d->lastPosition = -1;
}

bool DefaultHighlighter::skipHighlighting(const QString& text)
{
    return text.isEmpty();
}

void DefaultHighlighter::highlightBlock(const QString& text)
{
    kDebug() << text;
    const QTextCursor& cursor = d->cursor;
    d->lastBlockNumber = cursor.blockNumber();

    if (skipHighlighting(text))
        return;

    highlightPairs(text);
    highlightWords(text);
    highlightRegExps(text);
}

void DefaultHighlighter::addPair(const QChar& openSymbol, const QChar& closeSymbol)
{
    Q_ASSERT(!d->pairs.contains(openSymbol));
    Q_ASSERT(!d->pairs.contains(closeSymbol));
    d->pairs << openSymbol << closeSymbol;
}

void DefaultHighlighter::highlightPairs(const QString& text)
{
    kDebug() << text;
    const QTextCursor& cursor = d->cursor;
    int cursorPos = -1;
    if (cursor.blockNumber() == currentBlock().blockNumber() ) {
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

void DefaultHighlighter::highlightWords(const QString& text)
{
    kDebug() << "DefaultHighlighter::highlightWords";

    const QStringList& words = text.split(QRegExp("\\b"), QString::SkipEmptyParts);
    int count;
    int pos = 0;

    const int n = words.size();
    for (int i = 0; i < n; ++i)
    {
        count = words[i].size();
        QString word = words[i];

        //kind of a HACK:
        //look at previous words, if they end with allowed characters,
        //prepend them to the current word. This allows for example
        //to highlight words that start with a "Non-word"-character
        //e.g. %pi in the scilab backend.
        kDebug() << "nonSeparatingCharacters().isNull(): " << nonSeparatingCharacters().isNull();
        if(!nonSeparatingCharacters().isNull())
        {
            for(int j = i - 1; j >= 0; j--)
            {
                kDebug() << "j: " << j << "w: " << words[j];
                const QString& w = words[j];
                const QString exp = QString("(%1)*$").arg(nonSeparatingCharacters());
                kDebug() << "exp: " << exp;
                int idx = w.indexOf(QRegExp(exp));
                const QString& s = w.mid(idx);
                kDebug() << "s: " << s;

                if(s.size() > 0)
                {
                    pos -= s.size();
                    count += s.size();
                    word = s + word;
                } else{
                    break;
                }
            }
        }

        word = word.trimmed();

        kDebug() << "highlighing: " << word;

        if (d->wordRules.contains(word))
        {
            setFormat(pos, count, d->wordRules[word]);
        }

        pos += count;
    }
}

void DefaultHighlighter::highlightRegExps(const QString& text)
{
    foreach (const HighlightingRule& rule, d->regExpRules)
    {
        int index = rule.regExp.indexIn(text);
        while (index >= 0) {
            int length = rule.regExp.matchedLength();
            setFormat(index,  length,  rule.format);
            index = rule.regExp.indexIn(text,  index + length);
        }
    }
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


void DefaultHighlighter::positionChanged(QTextCursor cursor)
{
    if (!cursor.isNull() && cursor.document() != document())
	// A new item notified us, but we did not yet change our document.
	// We are waiting for that to happen.
	return;

    d->cursor = cursor;
    if ( (cursor.isNull() || cursor.blockNumber() != d->lastBlockNumber) &&
	 d->lastBlockNumber >= 0 ) {
        // remove highlight from last focused block
        rehighlightBlock(document()->findBlockByNumber(d->lastBlockNumber));
    }

    if (cursor.isNull()) {
	d->lastBlockNumber = -1;
	d->lastPosition = -1;
	return;
    }

    d->lastBlockNumber = cursor.blockNumber();

    if ( d->lastPosition == cursor.position() ) {
        return;
    }

    rehighlightBlock(cursor.block());
    d->lastPosition = cursor.position();
}

void DefaultHighlighter::addRule(const QString& word, const QTextCharFormat& format)
{
    d->wordRules[word] = format;
}

void DefaultHighlighter::addRule(const QRegExp& regexp, const QTextCharFormat& format)
{
    HighlightingRule rule = { regexp, format };
    d->regExpRules.removeAll(rule);
    d->regExpRules.append(rule);
}

void DefaultHighlighter::removeRule(const QString& word)
{
    d->wordRules.remove(word);
}

void DefaultHighlighter::removeRule(const QRegExp& regexp)
{
    HighlightingRule rule = { regexp, QTextCharFormat() };
    d->regExpRules.removeAll(rule);
}

QString DefaultHighlighter::nonSeparatingCharacters() const
{
    return QString();
}

#include  "defaulthighlighter.moc"
