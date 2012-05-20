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

class QGraphicsTextItem;

namespace Cantor
{
class DefaultHighlighterPrivate;

/**
 * The DefaultHighlighter is an implementation QSyntaxHighlighter.
 * It covers most common cases of syntax highlighting for Cantor's command entries.
 *
 * When creating a custom highlighter, for example for a new backend, you should use
 * the provided functions addPairs(), addRule() and/or addRules().
 *
 * If you need more specific functionality, subclass highlightBlock(). Usually it's a good idea to also call
 * DefaultHighlighter's implementation from it.
 *
 * @author Alexander Rieder
 */

class CANTOR_EXPORT DefaultHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT
  public:
    DefaultHighlighter(QObject* parent);
    ~DefaultHighlighter();

    /**
     * Change the item beeing highlighted.
     */
    void setTextItem(QGraphicsTextItem* item);

  public slots:
    /**
     * Called when the cursor moved. Rehighlights accordingly.
     */
    void positionChanged(QTextCursor);

  protected:
    /**
     * This method is called by Cantor's KTextEdit and is where all the highlighting must take place.
     * The default implementation calls highlightPairs(), highlightWords() and highlightRegExps().
     *
     */
    virtual void highlightBlock(const QString& text);

    bool skipHighlighting(const QString& text);

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

    /**
     * Call this to add a pair of symbols for highlighting.
     * The default implementation of the class already adds (), {} and [], so no need to add those.
     * For example, if you wanted to highlight angle-brackets, you would use:
     * @code
     * addPair('<', '>');
     * @endcode
     * @param openSymbol the opening symbol of the pair
     * @param closeSymbol the closing symbol of the pair
     * @sa highlightPairs
     */
    void addPair(const QChar& openSymbol, const QChar& closeSymbol);
    /**
     * Highlights all instances of the @p word in the text with the specified @p format
     * @param word the word to highlight
     * @param format the format to be used for displaying the word
     */
    void addRule(const QString& word, const QTextCharFormat& format);
    /**
     * Highlights all parts of the text matched by the regular expression @p regexp in the text
     * with the specified @p format
     * @param regexp the regular expression used to look for matches
     * @param format the format used to display the matching parts of the text
     */
    void addRule(const QRegExp& regexp, const QTextCharFormat& format);

    /**
     * Convenience method, highlights all items in @p conditions with the specified @p format
     * @code
     * QStringList greenWords;
     * greenWords << "tree" << "forest" << "grass";
     * addRules(greenWords, greenWordFormat);
     * @endcode
     * @param conditions any Qt container of QRegExp or QString.
     */
    template <class Container> void addRules(const Container& conditions, const QTextCharFormat& format);
    /**
     * Convenience method, equivalent to @code addRules(functions, functionFormat()) @endcode
     */
    template <class Container> void addFunctions(const Container& functions);
    /**
     * Convenience method, equivalent to @code addRules(variables, variableFormat()) @endcode
     */
    template <class Container> void addVariables(const Container& variables);
    /**
     * Convenience method, equivalent to @code addRules(keywords, keywordFormat()) @endcode
     */
    template <class Container> void addKeywords(const Container& keywords);

    /**
     * Removes any rules previously added for the word @p word
     */
    void removeRule(const QString& word);
    /**
     * Removes any rules previously added for the regular expression @p regexp
     */
    void removeRule(const QRegExp& regexp);
    /**
     * Convenience method, removes all rules with conditions from @p conditions
     * @sa removeRule, addRules
     */
    template <class Container> void removeRules(const Container& conditions);

    /**
     * Highlight pairs added with addPair()
     * @sa addPair
     */
    void highlightPairs(const QString& text);
    /**
     * Highlights words added with addRule()
     * @sa addRule, addRules
     */
    void highlightWords(const QString& text);
    /**
     * Highlights all matches from regular expressions added with addRule()
     * @sa addRule, addRules
     */
    void highlightRegExps(const QString& text);

    /**
     * Returns a string  that contains a regular expression that matches for characters thar are allowed inside
     * words for this backend. For example, maxima or scilab allow % at the beginning of variable names
     */
    virtual QString nonSeparatingCharacters() const;

  private slots:
    void updateFormats();

  private:
    DefaultHighlighterPrivate* d;
};


template <class Container>
void DefaultHighlighter::addRules(const Container& conditions, const QTextCharFormat& format)
{
    typename Container::const_iterator i = conditions.constBegin();
    typename Container::const_iterator end = conditions.constEnd();
    for (;i != end; ++i)
    {
        addRule(*i, format);
    }
}

template <class Container>
void DefaultHighlighter::addFunctions(const Container& functions)
{
    addRules(functions, functionFormat());
}

template <class Container>
void DefaultHighlighter::addKeywords(const Container& keywords)
{
    addRules(keywords, keywordFormat());
}

template <class Container>
void DefaultHighlighter::addVariables(const Container& variables)
{
    addRules(variables, variableFormat());
}

template <class Container>
void DefaultHighlighter::removeRules(const Container& conditions)
{
    typename Container::const_iterator i = conditions.constBegin();
    typename Container::const_iterator end = conditions.constEnd();
    for (;i != end; ++i)
    {
        removeRule(*i);
    }
}

}

#endif
