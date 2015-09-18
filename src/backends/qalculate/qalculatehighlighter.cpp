/************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                             *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "qalculatehighlighter.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>
#include <libqalculate/Unit.h>

#include <KColorScheme>
#include <KLocalizedString>
#include <QLocale>
#include <QDebug>

QalculateHighlighter::QalculateHighlighter(QObject* parent)
    : Cantor::DefaultHighlighter(parent)
{
}

QalculateHighlighter::~QalculateHighlighter()
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
    const QStringList& words = text.split(QRegExp(QLatin1String("\\b")), QString::SkipEmptyParts);

    qDebug() << "highlight block:" << text;

    CALCULATOR->beginTemporaryStopMessages();

    const QString decimalSymbol = QLocale().decimalPoint();

    for ( int i = 0; i < words.size(); ++i, pos += count ) {
        count = words[i].size();
        if ( words[i].trimmed().isEmpty() ) {
            continue;
        }

        qDebug() << "highlight word:" << words[i];

        QTextCharFormat format = errorFormat();

        if ( i < words.size() - 1 && words[i+1].trimmed() == QLatin1String("(") && CALCULATOR->getFunction(words[i].toUtf8().constData()) ) {
            // should be a function
            qDebug() << "function";
            format = functionFormat();
        } else if ( isOperatorAndWhitespace(words[i]) ) {
            // stuff like ") * (" is an invalid expression, but acutally OK

            // check if last number is actually a float
            bool isFloat = false;
            if ( words[i].trimmed() == decimalSymbol ) {
                if ( i > 0 ) {
                    // lookbehind
                    QString lastWord = words[i-1].trimmed();
                    if ( !lastWord.isEmpty() && lastWord.at(lastWord.size()-1).isNumber() ) {
                        qDebug() << "actually float";
                        isFloat = true;
                    }
                }
                if ( !isFloat && i < words.size() - 1 ) {
                    // lookahead
                    QString nextWord = words[i+1].trimmed();
                    if ( !nextWord.isEmpty() && nextWord.at(0).isNumber() ) {
                        qDebug() << "float coming";
                        isFloat = true;
                    }
                }
            }
            if ( !isFloat ) {
                qDebug() << "operator / whitespace";
                format = operatorFormat();
            } else {
                format = numberFormat();
            }
        } else {
            MathStructure expr = CALCULATOR->parse(words[i].toLatin1().constData());
            if ( expr.isNumber() || expr.isNumber_exp() ) {
                qDebug() << "number";
                format = numberFormat();
            } else if ( expr.isVariable() ) {
                qDebug() << "variable";
                format = variableFormat();
            } else if ( expr.isUndefined() ) {
                qDebug() << "undefined";
            } else if ( expr.isUnit() || expr.isUnit_exp() ) {
                qDebug() << "unit";
                format = keywordFormat();
            }
        }

        setFormat(pos, count, format);
    }

    CALCULATOR->endTemporaryStopMessages();

}

bool QalculateHighlighter::isOperatorAndWhitespace(const QString& word) const
{
    foreach ( const QChar& c, word ) {
        if ( c.isLetterOrNumber() ) {
            return false;
        }
    }
    return true;
}
