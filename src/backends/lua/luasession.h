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
    Copyright (C) 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
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
    LuaSession( Cantor::Backend* backend);
    ~LuaSession() override = default;

    void login() override;
    void logout() override;

    void interrupt() override;

    void runExpression(LuaExpression* currentExpression);

    Cantor::Expression*         evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave) override;
    Cantor::CompletionObject*   completionFor(const QString& cmd, int index=-1) override;
    QSyntaxHighlighter* syntaxHighlighter(QObject* parent) override;
    lua_State*                  getState() const;

public Q_SLOTS:
    void readIntroMessage();
    void readOutput();
    void readError();
    void processStarted();

private Q_SLOTS:
    void expressionFinished(Cantor::Expression::Status status);

private:
    lua_State* m_L;
    QProcess* m_process;
    LuaExpression* m_currentExpression;
    QString m_output;
};

#endif /* _LUASESSION_H */
