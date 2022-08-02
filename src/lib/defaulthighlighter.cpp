/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>
*/

#include "defaulthighlighter.h"
#include "session.h"

#include <QApplication>
#include <QTextDocument>
#include <QTextCursor>
#include <QGraphicsTextItem>
#include <KColorScheme>
#include <QStack>

using namespace Cantor;

struct HighlightingRule
{
    QRegularExpression regExp;
    QTextCharFormat format;
};

bool operator==(const HighlightingRule& rule1, const HighlightingRule& rule2)
{
    return rule1.regExp == rule2.regExp;
}

struct PairOpener {
    PairOpener() : position(-1), type(-1) { }
    PairOpener(int p, int t) : position(p), type(t) { }

    int position;
    int type;
};

class Cantor::DefaultHighlighterPrivate
{
  public:
    QTextCursor cursor;

    //Character formats to use for the highlighting
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
    QTextCharFormat mismatchingPairFormat;

    int lastBlockNumber = -1;
    int lastPosition = -1;
    bool suppressRuleChangedSignal = false;

    // each two consecutive items build a pair
    QList<QChar> pairs;

    QList<HighlightingRule> regExpRules;
    QHash<QString, QTextCharFormat> wordRules;
};

DefaultHighlighter::DefaultHighlighter(QObject* parent) : QSyntaxHighlighter(parent),
    d(new DefaultHighlighterPrivate)
{
    addPair(QLatin1Char('('), QLatin1Char(')'));
    addPair(QLatin1Char('['), QLatin1Char(']'));
    addPair(QLatin1Char('{'), QLatin1Char('}'));

    updateFormats();
    connect(qApp, &QGuiApplication::paletteChanged,
            this, &DefaultHighlighter::updateFormats);
}

DefaultHighlighter::DefaultHighlighter(QObject* parent, Session* session)
    :DefaultHighlighter(parent)
{
    if (session)
    {
        auto* model = session->variableModel();
        if (model)
        {
            connect(model, &DefaultVariableModel::variablesAdded, this, &DefaultHighlighter::addVariables);
            connect(model, &DefaultVariableModel::variablesRemoved, this, &DefaultHighlighter::removeRules);
            connect(model, &DefaultVariableModel::functionsAdded, this, &DefaultHighlighter::addFunctions);
            connect(model, &DefaultVariableModel::functionsRemoved, this, &DefaultHighlighter::removeRules);

            addVariables(model->variableNames());
            addFunctions(model->functions());
        }
    }
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
    d->lastBlockNumber = d->cursor.blockNumber();
    if (skipHighlighting(text))
        return;

    highlightPairs(text);
    highlightWords(text);
    highlightRegExps(text);
}

void DefaultHighlighter::addPair(QChar openSymbol, QChar closeSymbol)
{
    Q_ASSERT(!d->pairs.contains(openSymbol));
    Q_ASSERT(!d->pairs.contains(closeSymbol));
    d->pairs << openSymbol << closeSymbol;
}
void DefaultHighlighter::highlightPairs(const QString& text)
{
    const auto& cursor = d->cursor;
    int cursorPos = -1;
    if (cursor.blockNumber() == currentBlock().blockNumber() ) {
        cursorPos = cursor.position() - currentBlock().position();
        // when text changes, this will be called before the positionChanged signal
        // gets emitted. Hence update the position so we don't highlight twice
        d->lastPosition = cursor.position();
    }

    QStack<PairOpener> opened;

    for (int i = 0; i < text.size(); ++i) {
        int idx = d->pairs.indexOf(text[i]);
        if (idx == -1)
            continue;
        if (idx % 2 == 0) { //opener of a pair
            opened.push(PairOpener(i, idx));
        } else if (opened.isEmpty()) { //closer with no previous opener
            setFormat(i, 1, errorFormat());
        } else if (opened.top().type == idx - 1) { //closer with matched opener
            int openPos = opened.pop().position;
            if  (cursorPos != -1 &&
                (openPos == cursorPos || openPos == cursorPos - 1 ||
                i == cursorPos || i == cursorPos - 1)) {
                setFormat(openPos, 1, matchingPairFormat());
                setFormat(i, 1, matchingPairFormat());
            }
        } else { //closer with mismatching opener
            int openPos = opened.pop().position;
            setFormat(openPos, 1, mismatchingPairFormat());
            setFormat(i, 1, mismatchingPairFormat());
        }
    }

    // handled unterminated pairs
    while (!opened.isEmpty()) {
        int position = opened.pop().position;
        setFormat(position, 1, errorFormat());
    }
}

QStringList Cantor::DefaultHighlighter::parseBlockTextToWords(const QString& text)
{
    return text.split(QRegularExpression(QStringLiteral("\\b")), QString::SkipEmptyParts);
}

void DefaultHighlighter::highlightWords(const QString& text)
{
    //qDebug() << "DefaultHighlighter::highlightWords";

    const QStringList& words = parseBlockTextToWords(text);
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
        //qDebug() << "nonSeparatingCharacters().isNull(): " << nonSeparatingCharacters().isNull();
        if(!nonSeparatingCharacters().isNull())
        {
            for(int j = i - 1; j >= 0; j--)
            {
                //qDebug() << "j: " << j << "w: " << words[j];
                const QString& w = words[j];
                const QString exp = QStringLiteral("(%1)*$").arg(nonSeparatingCharacters());
                //qDebug() << "exp: " << exp;
                int idx = w.indexOf(QRegularExpression(exp));
                const QString& s = w.mid(idx);
                //qDebug() << "s: " << s;

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

        //qDebug() << "highlighing: " << word;

        if (d->wordRules.contains(word))
        {
            setFormat(pos, count, d->wordRules[word]);
        }

        pos += count;
    }
}

void DefaultHighlighter::highlightRegExps(const QString& text)
{
    for (const auto& rule : d->regExpRules)
    {
        auto iter = rule.regExp.globalMatch(text);
        while (iter.hasNext()) {
            auto match = iter.next();
            setFormat(match.capturedStart(0), match.capturedLength(0), rule.format);
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

QTextCharFormat DefaultHighlighter::mismatchingPairFormat() const
{
    return d->mismatchingPairFormat;
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

    d->mismatchingPairFormat.setForeground(scheme.foreground(KColorScheme::NegativeText));
    d->mismatchingPairFormat.setBackground(scheme.background(KColorScheme::NegativeBackground));
}


void DefaultHighlighter::positionChanged(const QTextCursor& cursor)
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
    if (!d->suppressRuleChangedSignal)
        emit rulesChanged();
}

void DefaultHighlighter::addRule(const QRegularExpression& regexp, const QTextCharFormat& format)
{
    HighlightingRule rule = { regexp, format };
    d->regExpRules.removeAll(rule);
    d->regExpRules.append(rule);
    if (!d->suppressRuleChangedSignal)
        emit rulesChanged();
}

void DefaultHighlighter::removeRule(const QString& word)
{
    d->wordRules.remove(word);
    if (!d->suppressRuleChangedSignal)
        emit rulesChanged();
}

void DefaultHighlighter::removeRule(const QRegularExpression& regexp)
{
    HighlightingRule rule = { regexp, QTextCharFormat() };
    d->regExpRules.removeAll(rule);
    if (!d->suppressRuleChangedSignal)
        emit rulesChanged();
}

void DefaultHighlighter::addRules(const QStringList& conditions, const QTextCharFormat& format)
{
    auto i = conditions.constBegin();
    d->suppressRuleChangedSignal = true;
    for (;i != conditions.constEnd(); ++i)
    {
        addRule(*i, format);
    }
    d->suppressRuleChangedSignal = false;
    emit rulesChanged();
}

void DefaultHighlighter::addFunctions(const QStringList& functions)
{
    addRules(functions, functionFormat());
}

void DefaultHighlighter::addKeywords(const QStringList& keywords)
{
    addRules(keywords, keywordFormat());
}

void DefaultHighlighter::addVariables(const QStringList& variables)
{
    addRules(variables, variableFormat());
}

void DefaultHighlighter::removeRules(const QStringList& conditions)
{
    auto i = conditions.constBegin();
    d->suppressRuleChangedSignal = true;
    for (;i != conditions.constEnd(); ++i)
    {
        removeRule(*i);
    }
    d->suppressRuleChangedSignal = false;
    emit rulesChanged();
}

QString DefaultHighlighter::nonSeparatingCharacters() const
{
    return QString();
}
