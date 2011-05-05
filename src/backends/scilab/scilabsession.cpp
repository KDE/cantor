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
    Copyright (C) 2011 Filipe Saraiva <filip.saraiva@gmail.com>
 */

#include "scilabsession.h"
#include "scilabexpression.h"
// #include "scilabcompletionobject.h"

#include <kdebug.h>

ScilabSession::ScilabSession( Cantor::Backend* backend) : Session(backend)
{
    kDebug();
}

ScilabSession::~ScilabSession()
{
    kDebug();
}

void ScilabSession::login()
{
    kDebug()<<"login";
    changeStatus(Cantor::Session::Done);
    emit ready();
}

void ScilabSession::logout()
{
    kDebug()<<"logout";
}

void ScilabSession::interrupt()
{
    kDebug()<<"interrupt";
    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();
    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* ScilabSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug()<<"evaluating: "<<cmd;
    ScilabExpression* expr=new ScilabExpression(this);
    expr->setFinishingBehavior(behave);
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(expressionFinished()));
    expr->setCommand(cmd);
    expr->evaluate();

    if(m_runningExpressions.isEmpty())
        changeStatus(Cantor::Session::Running);
    m_runningExpressions.append(expr);


    return expr;
}

// Cantor::CompletionObject* ScilabSession::completionFor(const QString& command)
// {
//     kDebug()<<"tab completion for "<<command;
//     return new ScilabCompletionObject(command, this);
// }

void ScilabSession::expressionFinished()
{
    kDebug()<<"finished";
    ScilabExpression* expression=qobject_cast<ScilabExpression*>(sender());
    m_runningExpressions.removeAll(expression);
    kDebug()<<"size: "<<m_runningExpressions.size();

    if(m_runningExpressions.isEmpty())
       changeStatus(Cantor::Session::Done);
}

#include "scilabsession.moc"
