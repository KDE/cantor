/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
*/

#include "luaexpression.h"
#include "luasession.h"
#include "luahelper.h"

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"

#include <lua.hpp>

#include <QDebug>
#include <QString>
#include <QStringList>

LuaExpression::LuaExpression(Cantor::Session* session, bool internal)
    : Cantor::Expression(session, internal)
{
}

void LuaExpression::evaluate()
{
    /*
     * start evaluating the current expression
     * set the status to computing
     * decide what needs to be done if the user is trying to define a function etc
    */
    if (command().isEmpty()) {
        setStatus(Cantor::Expression::Done);
        return;
    }

    session()->enqueueExpression(this);
}

void LuaExpression::parseError(const QString &error)
{
    qDebug() << error;
    setErrorMessage(error);
    setStatus(Error);
}

void LuaExpression::parseOutput(const QString& output)
{

    qDebug()<<"parsing the output " << output;
    auto* luaSession = static_cast<LuaSession*>(session());

    if (luaSession->isLuaJIT())
    {
        QString result = output;

        // in case the expression is incomplete, Lua is answering with the sub-promt ">> ".
        // since we don't handle it yet, replace it with the prompt string so we can handle it easier below
        // when splitting the whole output into the separate results
        // TODO: add handling for the sub-promt
        result.replace(QLatin1String(">> "), QLatin1String("> "));

        const auto& results = result.split(QLatin1String("> "));
        for (auto& result : results) {
            if (result.simplified() == QLatin1String(">") || result.simplified().isEmpty())
                continue;

            addResult(new Cantor::TextResult(result));
        }
    }
    else
    {
        // the parsing of Lua's output was already done in LuaSession::readOutputLua()
        // where the information about the actual commands is present and required.
        // here we only set the final result without any further parsing.
        if (!output.isEmpty())
            setResult(new Cantor::TextResult(output));
    }

    setStatus(Cantor::Expression::Done);
}
