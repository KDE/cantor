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
using namespace MathematiK;

#include "backend.h"

class MathematiK::SessionPrivate
{
  public:
    SessionPrivate()
    {
        backend=0;
        expressionCount=0;
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

Backend* Session::backend()
{
    return d->backend;
}

MathematiK::Session::Status Session::status()
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

TabCompletionObject* Session::tabCompletionFor(const QString& cmd)
{
    Q_UNUSED(cmd);
    //Return 0 per default, so Backends not offering tab completions don't have
    //to reimplement this. This method should only be called on backends with
    //the TabCompletion Capability flag

    return 0;
}

int Session::nextExpressionId()
{
    return d->expressionCount++;
}

#include "session.moc"

