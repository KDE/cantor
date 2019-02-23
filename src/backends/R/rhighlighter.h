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
    Copyright (C) 2010 Oleksiy Protas <elfy.ua@gmail.com>
 */

#ifndef _RHIGHLIGHTER_H
#define _RHIGHLIGHTER_H

#include "defaulthighlighter.h"

class RHighlighter : public Cantor::DefaultHighlighter
{
  Q_OBJECT

  public:
    explicit RHighlighter( QObject* parent);
    ~RHighlighter() override = default;

  protected:
    void highlightBlock(const QString &text) override;

  public Q_SLOTS:
    void addUserVariable(const QStringList& vars);
    void removeUserVariable(const QStringList& vars);
    void addUserFunction(const QStringList& funcs);
    void removeUserFunction(const QStringList& funcs);

  private:
    inline void formatRule(const QRegExp &p, const QTextCharFormat &fmt, const QString& text,bool shift=false);
    inline void massFormat(const QVector<QRegExp>& rules, const QTextCharFormat &fmt, const QString& text,bool shift=false);

    void addUserDefinition(const QStringList& names, QVector<QRegExp>& vector);
    void removeUserDefinition(const QStringList& names, QVector<QRegExp>& vector);

    static const QStringList operators_list;
    static const QStringList specials_list;
    QVector<QRegExp> operators;
    QVector<QRegExp> specials;
    QVector<QRegExp> functions;
    QVector<QRegExp> variables;
};

#endif /* _RHIGHLIGHTER_H */
