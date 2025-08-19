/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
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

void LuaExpression::parseError(const QString &error)
{
    m_errorBuffer.append(error);
}

void LuaExpression::parseOutput(const QString& output)
{
    if (!m_errorBuffer.isEmpty())
    {
        setErrorMessage(m_errorBuffer.trimmed());
        setStatus(Error);
    }
    else
    {
        qDebug()<<"parsing the output " << output;
        auto* luaSession = static_cast<LuaSession*>(session());

        if (luaSession->isLuaJIT())
        {
            const auto& lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

            for (const QString& line : lines)
            {
                QString simplifiedLine = line.simplified();
                if (simplifiedLine.startsWith(QLatin1String(">")))
                {
                    continue;
                }

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
}

