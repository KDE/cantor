/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
*/

#ifndef _RHIGHLIGHTER_H
#define _RHIGHLIGHTER_H

#include "defaulthighlighter.h"

#include <QRegularExpression>

class RSession;

class RHighlighter : public Cantor::DefaultHighlighter
{
  Q_OBJECT

  public:
    explicit RHighlighter( QObject* parent, RSession* session);
    ~RHighlighter() override = default;

  protected:
    QStringList parseBlockTextToWords(const QString& text) override;

  private:
    inline void formatRule(const QRegularExpression &p, const QTextCharFormat &fmt, const QString& text,bool shift=false);
    inline void massFormat(const QVector<QRegularExpression>& rules, const QTextCharFormat &fmt, const QString& text,bool shift=false);

    static const QStringList operators_list;
    static const QStringList specials_list;
    QVector<QRegularExpression> operators;
    QVector<QRegularExpression> specials;
    QVector<QRegularExpression> functions;
    QVector<QRegularExpression> variables;
};

#endif /* _RHIGHLIGHTER_H */
