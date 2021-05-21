/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _MAXIMAHIGHLIGHTER_H
#define _MAXIMAHIGHLIGHTER_H

#include <QRegularExpression>

#include "defaulthighlighter.h"
class MaximaSession;

class MaximaHighlighter : public Cantor::DefaultHighlighter
{
  Q_OBJECT
  public:
    MaximaHighlighter( QObject* parent, MaximaSession* session);
    ~MaximaHighlighter() override = default;

  protected:
    void highlightBlock(const QString &text) override;

    QString nonSeparatingCharacters() const override;

  private:
     QRegularExpression commentStartExpression;
     QRegularExpression commentEndExpression;
};

#endif /* _MAXIMAHIGHLIGHTER_H */
