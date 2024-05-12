/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qalculatehighlighter.h"
#include <libqalculate/Calculator.h>
#include <KLocalizedString>
#include <QLocale>
#include <QRegularExpression>

QalculateHighlighter::QalculateHighlighter(QObject* parent)
    : Cantor::DefaultHighlighter(parent)
{
}

void QalculateHighlighter::highlightBlock(const QString& text)
{
    if ( text.isEmpty() || text.trimmed().isEmpty() || text.startsWith(QLatin1String(">>> "))
            // filter error messages, they get highlighted via html
            || text.startsWith(i18n("ERROR") + QLatin1Char(':')) || text.startsWith(i18n("WARNING") + QLatin1Char(':')) ) {
        return;
    }

    int pos = 0;
    int count;
    ///TODO: Can't we use CALCULATOR->parse() or similar?
    ///      Question is how to get the connection between
    ///      MathStructur and position+length in @p text
    const QStringList words = text.split(QRegularExpression(QStringLiteral("\\b")), QString::SkipEmptyParts);

    // qDebug() << "highlight block:" << text;

    CALCULATOR->beginTemporaryStopMessages();

    const QString decimalSymbol = QLocale().decimalPoint();

    for ( int i = 0; i < words.size(); ++i, pos += count ) {
        count = words[i].size();
        if ( words[i].trimmed().isEmpty() ) {
            continue;
        }

        auto format = errorFormat();

        if ( i < words.size() - 1 && words[i+1].trimmed() == QLatin1String("(") && CALCULATOR->getFunction(words[i].toUtf8().constData()) ) {
            // should be a function
            format = functionFormat();
        } else if ( isOperatorAndWhitespace(words[i]) ) {
            // stuff like ") * (" is an invalid expression, but actually OK

            // check if last number is actually a float
            bool isFloat = false;
            if ( words[i].trimmed() == decimalSymbol ) {
                if ( i > 0 ) {
                    // lookbehind
                    QString lastWord = words[i-1].trimmed();
                    if ( !lastWord.isEmpty() && lastWord.at(lastWord.size()-1).isNumber() ) {
                        isFloat = true;
                    }
                }
                if ( !isFloat && i < words.size() - 1 ) {
                    // lookahead
                    QString nextWord = words[i+1].trimmed();
                    if ( !nextWord.isEmpty() && nextWord.at(0).isNumber() ) {
                        isFloat = true;
                    }
                }
            }
            if ( !isFloat ) {
                format = operatorFormat();
            } else {
                format = numberFormat();
            }
        } else {
            MathStructure expr = CALCULATOR->parse(words[i].toLatin1().constData());
            if ( expr.isNumber() || expr.isNumber_exp() ) {
                format = numberFormat();
            } else if ( expr.isVariable() ) {
                format = variableFormat();
            } else if ( expr.isUndefined() ) {
            } else if ( expr.isUnit() || expr.isUnit_exp() ) {
                format = keywordFormat();
            }
        }

        setFormat(pos, count, format);
    }

    CALCULATOR->endTemporaryStopMessages();
}

bool QalculateHighlighter::isOperatorAndWhitespace(const QString& word) const
{
    for (const auto& c : word ) {
        if ( c.isLetterOrNumber() )
            return false;
    }

    return true;
}
