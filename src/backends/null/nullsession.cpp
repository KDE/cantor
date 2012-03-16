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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "nullsession.h"
#include "nullexpression.h"
#include "nullcompletionobject.h"

#include <kdebug.h>

NullSession::NullSession( Cantor::Backend* backend) : Session(backend)
{
    kDebug();
}

NullSession::~NullSession()
{
    kDebug();
}

void NullSession::login()
{
    kDebug()<<"login";
    changeStatus(Cantor::Session::Done);
    emit ready();
}

void NullSession::logout()
{
    kDebug()<<"logout";
}

void NullSession::interrupt()
{
    kDebug()<<"interrupt";
    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();
    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* NullSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug()<<"evaluating: "<<cmd;
    NullExpression* expr=new NullExpression(this);
    expr->setFinishingBehavior(behave);
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionFinished()));
    expr->setCommand(cmd);
    expr->evaluate();

    if(m_runningExpressions.isEmpty())
        changeStatus(Cantor::Session::Running);
    m_runningExpressions.append(expr);


    return expr;
}

Cantor::CompletionObject* NullSession::completionFor(const QString& command, int index)
{
    kDebug()<<"tab completion for "<<command;
    return new NullCompletionObject(command, index, this);
}

void NullSession::expressionFinished()
{
    kDebug()<<"finished";
    NullExpression* expression=qobject_cast<NullExpression*>(sender());
    m_runningExpressions.removeAll(expression);
    kDebug()<<"size: "<<m_runningExpressions.size();

    if(m_runningExpressions.isEmpty())
       changeStatus(Cantor::Session::Done);
}

#include "nullsession.moc"
