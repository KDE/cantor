/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#pragma once

#include "defaulthighlighter.h"

class JuliaSession;

/**
 * Implementation of JuliaHighlighter
 *
 * Takes into account loaded symbols from scope and predefined keywords.
 * There is no common regexps that bound to fail with such syntax-overloaded
 * languages as Julia
 */
class JuliaHighlighter: public Cantor::DefaultHighlighter
{
    Q_OBJECT

public:
    /**
     * Constructs JuliaHighlighter
     *
     * @param parent QObject parent
     */
    explicit JuliaHighlighter(QObject *parent, JuliaSession* session);
    ~JuliaHighlighter() override = default;

protected:
    /**
     * @see Cantor::DefaultHighlighter::highlightBlock
     */
    void highlightBlock(const QString &text) override;

    /**
     * @see Cantor::DefaultHighlighter::nonSeparatingCharacters
     */
    QString nonSeparatingCharacters() const override;
};
