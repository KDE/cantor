/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
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
    ~JuliaCompletionObject() override;

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
