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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliaexpression.h"

#include "juliasession.h"
#include "textresult.h"

JuliaExpression::JuliaExpression(Cantor::Session *session)
    : Cantor::Expression(session)
{
}

void JuliaExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);
    dynamic_cast<JuliaSession *>(session())->runExpression(this);
}

void JuliaExpression::finalize()
{
    auto juliaSession = dynamic_cast<JuliaSession *>(session());
    setErrorMessage(
        juliaSession->getError()
            .replace(QLatin1String("\n"), QLatin1String("<br>"))
    );
    if (juliaSession->getWasException()) {
        setResult(new Cantor::TextResult(juliaSession->getOutput()));
        setStatus(Cantor::Expression::Error);
    } else {
        setResult(new Cantor::TextResult(juliaSession->getOutput()));
        setStatus(Cantor::Expression::Done);
    }
}

void JuliaExpression::interrupt()
{
    setStatus(Cantor::Expression::Interrupted);
}

#include "juliaexpression.moc"
