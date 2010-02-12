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

#include "completionobject.h"
using namespace Cantor;

#include <QStringList>
#include <QTimer>

#include "session.h"

class Cantor::CompletionObjectPrivate
{
  public:
    QStringList completions;
    QString command;
    Session* session;
};

CompletionObject::CompletionObject(const QString& command, Session* session) :
                                                                                     d(new CompletionObjectPrivate)
{
    setParent(session);
    d->command=command;
    d->session=session;
    setCompletionMode(KGlobalSettings::CompletionShell);

    //start a delayed fetch
    QTimer::singleShot(0, this, SLOT(fetchCompletions()));
}

CompletionObject::~CompletionObject()
{
    delete d;
}

QString CompletionObject::command() const
{
    return d->command;
}

Session* CompletionObject::session() const
{
    return d->session;
}

QStringList CompletionObject::completions() const
{
    return d->completions;
}

void CompletionObject::setCompletions(const QStringList& completions)
{
    d->completions=completions;
    foreach(const QString& comp, d->completions)
    {
        this->addItem(comp);
    }
}

void CompletionObject::setCommand(const QString& cmd)
{
    d->command=cmd;
}

#include "completionobject.moc"
