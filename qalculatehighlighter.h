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

#ifndef QALCULATEHIGHLIGHTER_H
#define QALCULATEHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>

class QalculateHighlighter : public QSyntaxHighlighter
{
public:
    QalculateHighlighter(QTextEdit* parent);
    ~QalculateHighlighter();

protected:
    virtual void highlightBlock(const QString& text);

private:
    bool isIdentifier(const QString &identifier);

    QTextCharFormat* m_errFormat;
    QTextCharFormat* m_funcFormat;
    QTextCharFormat* m_varFormat;
    QTextCharFormat* m_unitFormat;
    ///TODO:
    QTextCharFormat* m_numFormat;
    QTextCharFormat* m_opFormat;
};

#endif // QALCULATEHIGHLIGHTER_H
