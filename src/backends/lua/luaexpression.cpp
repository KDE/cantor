/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
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

LuaExpression::LuaExpression( Cantor::Session* session, bool internal)
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

void LuaExpression::parseError(QString &error)
{
    qDebug() << error;
    setErrorMessage(error);
    setStatus(Error);
}

void LuaExpression::parseOutput(const QString &output)
{
    qDebug()<<"parsing the output " << output;

    if (!output.isEmpty())
        setResult(new Cantor::TextResult(output));
    setStatus(Cantor::Expression::Done);
}

void LuaExpression::interrupt()
{
    setStatus(Cantor::Expression::Interrupted);
}
