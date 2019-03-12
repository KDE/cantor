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

#include "completionobject.h"
#include "expression.h"

class JuliaSession;

/**
 * Implements code completion for Julia language
 *
 * Uses Julia's Base.REPL.REPLCompletions.completions command to get
 * context-aware completions like in native Julia REPL
 */
class JuliaCompletionObject : public Cantor::CompletionObject
{
public:
    /**
     * Constructs JuliaCompletionObject
     *
     * @param cmd command piece to generate completion
     * @param index index of cursor in command
     * @param session current session
     */
    JuliaCompletionObject(const QString &cmd, int index, JuliaSession *session);
    ~JuliaCompletionObject() override = default;

protected:
    /**
     * @see Cantor::CompletionObject::mayIdentifierContain
     */
    bool mayIdentifierContain(QChar c) const override;
    bool mayIdentifierBeginWith(QChar c) const override;

    /**
     * @see Cantor::CompletionObject::mayIdentifierBeginWith
     */

protected Q_SLOTS:
    /**
     * @see Cantor::CompletionObject::fetchCompletions
     */
    void fetchCompletions() override;
    void extractCompletions(Cantor::Expression::Status status);

private:
    Cantor::Expression* m_expression;
};
