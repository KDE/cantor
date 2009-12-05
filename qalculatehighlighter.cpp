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
    : QSyntaxHighlighter(parent),
    m_errFormat(new QTextCharFormat), m_funcFormat(new QTextCharFormat),
    m_varFormat(new QTextCharFormat), m_unitFormat(new QTextCharFormat),
    m_numFormat(new QTextCharFormat), m_opFormat(new QTextCharFormat)
{
    KColorScheme scheme(QPalette::Active);

    ///TODO: what happens when the global KDE style changes
    m_varFormat->setForeground(scheme.foreground(KColorScheme::ActiveText));

    m_funcFormat->setForeground(scheme.foreground(KColorScheme::LinkText));
    m_funcFormat->setFontWeight(QFont::DemiBold);

    m_unitFormat->setForeground(scheme.foreground(KColorScheme::PositiveText));
    m_unitFormat->setFontItalic(true);

    m_numFormat->setForeground(scheme.foreground(KColorScheme::NeutralText));

    m_errFormat->setForeground(scheme.foreground(KColorScheme::NormalText));
    m_errFormat->setUnderlineColor(scheme.foreground(KColorScheme::NegativeText).color());
    m_errFormat->setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}

QalculateHighlighter::~QalculateHighlighter()
{
}

void QalculateHighlighter::highlightBlock(const QString& text)
{
    int pos = 0;
    int count;
    QTextCharFormat* format;

    ///TODO: Can't we use CALCULATOR->parse() or similar?
    ///      Question is how to get the connection between
    ///      MathStructur and position+length in @p text
    const QStringList& words = text.split(QRegExp("\\b"), QString::SkipEmptyParts);

    CALCULATOR->beginTemporaryStopMessages();

    for ( int i = 0; i < words.size(); ++i, pos += count ) {
        count = words[i].size();

        kDebug() << words[i];

        format = 0;
        string needle(words[i].trimmed().toUtf8().constData());

        if ( i < words.size() - 1 && words[i+1].trimmed() == "(" && CALCULATOR->getFunction(needle) ) {
            // should be a function
            kDebug() << "function";
            format = m_funcFormat;
        } else {
            MathStructure expr = CALCULATOR->parse(words[i].toAscii().constData());
            if ( expr.isNumber() || expr.isNumber_exp() ) {
                kDebug() << "number";
                format = m_numFormat;
            } else if ( expr.isVariable() ) {
                kDebug() << "variable";
                format = m_varFormat;
            } else if ( expr.isUndefined() ) {
                kDebug() << "undefined";
                format = m_errFormat;
            } else if ( expr.isUnit() || expr.isUnit_exp() ) {
                kDebug() << "unit";
                format = m_unitFormat;
            }
        }

        if ( !format ) {
            kDebug() << "error";
            format = m_errFormat;
        }

        kDebug() << "setting format from" << pos << "to" << (pos+count) << '(' << words[i] << ')';
        setFormat(pos, count, *format);
    }

    CALCULATOR->endTemporaryStopMessages();

}


bool QalculateHighlighter::isIdentifier(const QString& identifier)
{
    if ( !identifier[0].isLetter() ) {
        return false;
    }
    for ( int i = 1; i < identifier.size(); ++i ) {
        if ( !identifier[i].isLetterOrNumber() ) {
            return false;
        }
    }
    return true;
}