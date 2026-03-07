/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
    SPDX-FileCopyrightText: 2023-2026 Alexander Semke <alexander.semke@web.de>
*/

#include "luaexpression.h"
#include "luasession.h"
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

void LuaExpression::parseOutput(const QString& output)
{
    qDebug()<<"parsing the output " << output;
    auto* luaSession = static_cast<LuaSession*>(session());

    if (luaSession->isLuaJIT())
    {
        const auto& lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

        for (QString line : lines)
        {
            while (true) {
                QString trimmed = line.trimmed();
                if (trimmed.startsWith(QLatin1Char('>'))) {
                    int idx = line.indexOf(QLatin1Char('>'));
                    line = line.mid(idx + 1);
                } else {
                    break;
                }
            }

            if (line.trimmed().isEmpty())
                continue;

            addResult(new Cantor::TextResult(line));
        }
    }
    else
    {
        if (!output.isEmpty())
            setResult(new Cantor::TextResult(output));
    }
    setStatus(Cantor::Expression::Done);
}
