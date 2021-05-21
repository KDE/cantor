/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QALCULATEHIGHLIGHTER_H
#define QALCULATEHIGHLIGHTER_H

#include "defaulthighlighter.h"

class QalculateHighlighter : public Cantor::DefaultHighlighter
{
public:
    explicit QalculateHighlighter(QObject* parent);
    ~QalculateHighlighter() override = default;

protected:
    void highlightBlock(const QString& text) override;

private:
    bool isOperatorAndWhitespace(const QString &word) const;
};

#endif // QALCULATEHIGHLIGHTER_H
