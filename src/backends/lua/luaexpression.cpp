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

LuaExpression::LuaExpression( Cantor::Session* session)
    : Cantor::Expression(session)
{
}

void LuaExpression::evaluate()
{
    /*
     * start evaluating the current expression
     * set the status to computing
     * decide what needs to be done if the user is trying to define a function etc
    */
    setStatus(Cantor::Expression::Computing);
    if (command().isEmpty()) {
        setStatus(Cantor::Expression::Done);
        return;
    }

    LuaSession* currentSession = dynamic_cast<LuaSession*>(session());
        currentSession->runExpression(this);
}

void LuaExpression::parseError(QString &error)
{
    qDebug() << error;
    setErrorMessage(error);
    setStatus(Error);
}

void LuaExpression::parseOutput(QString &output)
{
    qDebug()<<"parsing the output " << output;

    const QStringList& inputs = command().split(QLatin1Char('\n'));
    const QStringList& outputs = output.split(QLatin1Char('\n'));
    QString parsedOutput;

    for (auto out : outputs) {
        //remove lua's promt characters if available
        if (out.startsWith(QLatin1String("> ")))
            out.remove(0, 2);
        else if (out.startsWith(QLatin1String(">> ")))
            out.remove(0, 3);

        //for multi-line inputs lua returs the inputs as part of the output
        //-> remove the input lines from the output.
        //since lua doesn't always seem to preserve the spaces, compare trimmed strings
        out = out.trimmed();
        bool found = false;
        for (auto in : inputs) {
            if (out == in.trimmed()) {
                found = true;
                break;
            }
        }

        if (!found) {
            if (!parsedOutput.isEmpty())
                parsedOutput += QLatin1Char('\n');
            parsedOutput += out;
        }
    }

    qDebug() << "final output of the command " <<  command() << ": " << parsedOutput << endl;

    if (!parsedOutput.isEmpty())
        setResult(new Cantor::TextResult(parsedOutput));
    setStatus(Cantor::Expression::Done);
}

void LuaExpression::interrupt()
{
    setStatus(Cantor::Expression::Interrupted);
}
