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
    Copyright (C) 2010 Oleksiy Protas <elfy.ua@gmail.com>
 */

#include "rcompletionobject.h"
#include "rsession.h"

RCompletionObject::RCompletionObject(const QString& command, RSession* session) : Cantor::CompletionObject(command, session)
{
    setCommand(command);
    // STUB
}

RCompletionObject::~RCompletionObject()
{
    emit goesOutOfScope(this);
}

void RCompletionObject::fetchCompletions()
{
    emit requestCompletion(command());
}

void RCompletionObject::receiveCompletions(const QString& token,const QStringList& options)
{
    /* Setting up both completion variants -and- the token R generously found for us */
    //setCommand(token);
    //setCompletions(options);

    // TODO: investigate the empty token problem
    /* Not so fast, evidently KCompletion requires a nonempty token, hence this stub */
    if (token.length()==0 && command().length()!=0)
    {
        /* Adding previous symbol to token, ugly but effective */
        QString lastchar(command()[command().length()-1]);
        setCommand(lastchar);
        setCompletions(QStringList(options).replaceInStrings(QRegExp("^"),lastchar));
    }
    else
    {
        setCommand(token);
        setCompletions(options);
    }

    emit done();
}

#include "rcompletionobject.moc"
