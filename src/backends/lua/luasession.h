/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _LUASESSION_H
#define _LUASESSION_H

#include "session.h"
#include <lua.hpp>

class LuaExpression;
class QProcess;

class LuaSession : public Cantor::Session
{
  Q_OBJECT

public:
    explicit LuaSession(Cantor::Backend*);
    ~LuaSession() override;

    void login() override;
    void logout() override;

    void interrupt() override;
    void runFirstExpression() override;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave = Cantor::Expression::FinishingBehavior::DoNotDelete, bool internal = false) override;
    Cantor::CompletionObject* completionFor(const QString& cmd, int index = -1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;

    bool isLuaJIT() const;
    lua_State* getState() const;

public Q_SLOTS:
    void readIntroMessage();
    void readOutput();
    void readError();
    void processStarted();

private Q_SLOTS:
    void expressionFinished(Cantor::Expression::Status);

private:
    void readOutputLua();
    void readOutputLuaJIT();
    bool isPromptString(const QString&);

    lua_State* m_L{nullptr};
    QProcess* m_process{nullptr};
    LuaExpression* m_lastExpression{nullptr};
    QStringList m_inputCommands;
    QStringList m_output;
    bool m_luaJIT{true};

    static const QString LUA_PROMPT;
    static const QString LUA_SUBPROMPT;
};

#endif /* _LUASESSION_H */
