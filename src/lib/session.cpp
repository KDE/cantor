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

#include "session.h"
using namespace Cantor;

#include "backend.h"

class Cantor::SessionPrivate
{
  public:
    SessionPrivate()
    {
        backend=0;
        expressionCount=0;
        typesettingEnabled=false;
    }

    Backend* backend;
    Session::Status status;
    bool typesettingEnabled;
    int expressionCount;
};

Session::Session( Backend* backend ) : QObject(backend),
                                       d(new SessionPrivate)
{
    d->backend=backend;
}

Session::~Session()
{
    delete d;
}

Expression* Session::evaluateExpression(const QString& command)
{
    return evaluateExpression(command, Expression::DoNotDelete);
}

Backend* Session::backend()
{
    return d->backend;
}

Cantor::Session::Status Session::status()
{
    return d->status;
}

void Session::changeStatus(Session::Status newStatus)
{
    d->status=newStatus;
    emit statusChanged(newStatus);
}

void Session::setTypesettingEnabled(bool enable)
{
    d->typesettingEnabled=enable;
}

bool Session::isTypesettingEnabled()
{
    return d->typesettingEnabled;
}

CompletionObject* Session::completionFor(const QString& cmd, int index)
{
    Q_UNUSED(cmd);
    Q_UNUSED(index);
    //Return 0 per default, so Backends not offering tab completions don't have
    //to reimplement this. This method should only be called on backends with
    //the Completion Capability flag

    return 0;
}

SyntaxHelpObject* Session::syntaxHelpFor(const QString& cmd)
{
    Q_UNUSED(cmd);

    //Return 0 per default, so Backends not offering tab completions don't have
    //to reimplement this. This method should only be called on backends with
    //the SyntaxHelp Capability flag
    return 0;
}

QSyntaxHighlighter* Session::syntaxHighlighter(QObject* parent)
{
    Q_UNUSED(parent);
    return 0;
}

QAbstractItemModel* Session::variableModel()
{
    //Return 0 per default, so Backends not offering variable management don't
    //have to reimplement this. This method should only be called on backends with
    //VariableManagement Capability flag
    return 0;
}

int Session::nextExpressionId()
{
    return d->expressionCount++;
}



