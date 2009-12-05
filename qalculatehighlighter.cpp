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
#include <KDebug>


QalculateHighlighter::QalculateHighlighter(QTextEdit* parent)
    : Cantor::DefaultHighlighter(parent)
{
}

QalculateHighlighter::~QalculateHighlighter()
{
}

void QalculateHighlighter::highlightBlock(const QString& text)
{
    int pos = 0;
    int count;
    ///TODO: Can't we use CALCULATOR->parse() or similar?
    ///      Question is how to get the connection between
    ///      MathStructur and position+length in @p text
    const QStringList& words = text.split(QRegExp("\\b"), QString::SkipEmptyParts);

    CALCULATOR->beginTemporaryStopMessages();

    for ( int i = 0; i < words.size(); ++i, pos += count ) {
        count = words[i].size();

        kDebug() << words[i];

        QTextCharFormat format = errorFormat();

        if ( i < words.size() - 1 && words[i+1].trimmed() == "(" && CALCULATOR->getFunction(words[i].toUtf8().constData()) ) {
            // should be a function
            kDebug() << "function";
            format = functionFormat();
        } else if ( isOperatorAndWhitespace(words[i]) ) {
            // stuff like ") * (" is an invalid expression, but acutally OK
            kDebug() << "operator / whitespace";
            format = operatorFormat();
        } else {
            MathStructure expr = CALCULATOR->parse(words[i].toAscii().constData());
            if ( expr.isNumber() || expr.isNumber_exp() ) {
                kDebug() << "number";
                format = numberFormat();
            } else if ( expr.isVariable() ) {
                kDebug() << "variable";
                format = variableFormat();
            } else if ( expr.isUndefined() ) {
                kDebug() << "undefined";
            } else if ( expr.isUnit() || expr.isUnit_exp() ) {
                kDebug() << "unit";
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
