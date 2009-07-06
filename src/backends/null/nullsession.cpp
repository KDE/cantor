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
#include "nulltabcompletionobject.h"

#include <kdebug.h>

NullSession::NullSession( MathematiK::Backend* backend) : Session(backend)
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
    changeStatus(MathematiK::Session::Done);
    emit ready();
}

void NullSession::logout()
{
    kDebug()<<"logout";
}

void NullSession::interrupt()
{
    kDebug()<<"interrupt";
    foreach(MathematiK::Expression* e, m_runningExpressions)
        e->interrupt();
    m_runningExpressions.clear();
    changeStatus(MathematiK::Session::Done);
}

MathematiK::Expression* NullSession::evaluateExpression(const QString& cmd)
{
    kDebug()<<"evaluating: "<<cmd;
    NullExpression* expr=new NullExpression(this);
    connect(expr, SIGNAL(statusChanged(MathematiK::Expression::Status)), this, SLOT(expressionFinished()));
    expr->setCommand(cmd);
    expr->evaluate();

    if(m_runningExpressions.isEmpty())
        changeStatus(MathematiK::Session::Running);
    m_runningExpressions.append(expr);


    return expr;
}

MathematiK::TabCompletionObject* NullSession::tabCompletionFor(const QString& command)
{
    kDebug()<<"tab completion for "<<command;
    return new NullTabCompletionObject(command, this);
}

void NullSession::expressionFinished()
{
    kDebug()<<"finished";
    m_runningExpressions.removeAll(qobject_cast<NullExpression*>(sender()));
    kDebug()<<"size: "<<m_runningExpressions.size();
    if(m_runningExpressions.isEmpty())
       changeStatus(MathematiK::Session::Done);
}

#include "nullsession.moc"
