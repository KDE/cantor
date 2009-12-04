/*************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "qalculatesession.h"
#include "qalculateexpression.h"
#include "qalculatetabcompletionobject.h"

#include <QTextEdit>

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>
#include <KDebug>

#include "qalculatesyntaxhelpobject.h"

QalculateSession::QalculateSession( Cantor::Backend* backend)
    : Session(backend)
{
}

QalculateSession::~QalculateSession()
{
}

void QalculateSession::login()
{
    changeStatus(Cantor::Session::Done);
    emit ready();
}

void QalculateSession::logout()
{
}

void QalculateSession::interrupt()
{
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* QalculateSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    QalculateExpression* expr = new QalculateExpression(this);
    expr->setFinishingBehavior(behave);

    changeStatus(Cantor::Session::Running);
    expr->setCommand(cmd);
    expr->evaluate();
    changeStatus(Cantor::Session::Done);

    return expr;
}

Cantor::TabCompletionObject* QalculateSession::tabCompletionFor(const QString& command)
{
    return new QalculateTabCompletionObject(command, this);
}

Cantor::SyntaxHelpObject* QalculateSession::syntaxHelpFor(const QString& cmd)
{
    return new QalculateSyntaxHelpObject(cmd, this);
}
