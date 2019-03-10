/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
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
