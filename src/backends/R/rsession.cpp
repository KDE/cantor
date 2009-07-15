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

#include "rsession.h"
#include "rexpression.h"
#include "rthread.h"

#include <QTimer>
#include <kdebug.h>

RSession::RSession( MathematiK::Backend* backend) : Session(backend)
{
    kDebug();

}

RSession::~RSession()
{
    kDebug();
}

void RSession::login()
{
    kDebug()<<"login";

    m_thread=new RThread(this);
    m_thread->start();
    connect(m_thread, SIGNAL(expressionFinished(RExpression*)), this, SLOT(expressionFinished()));

    changeStatus(MathematiK::Session::Done);
    QTimer::singleShot(0, this, SIGNAL(ready()));
}

void RSession::logout()
{
    kDebug()<<"logout";
    //m_thread->quit();
}

void RSession::interrupt()
{
    kDebug()<<"interrupt";

    changeStatus(MathematiK::Session::Done);
}

MathematiK::Expression* RSession::evaluateExpression(const QString& cmd)
{
    kDebug()<<"evaluating: "<<cmd;
    RExpression* expr=new RExpression(this);

    expr->setCommand(cmd);

    m_thread->queueExpression(expr);

    changeStatus(MathematiK::Session::Running);

    return expr;
}

void RSession::expressionFinished()
{
    if(!m_thread->isBusy())
        changeStatus(MathematiK::Session::Done);
}

#include "rsession.moc"
