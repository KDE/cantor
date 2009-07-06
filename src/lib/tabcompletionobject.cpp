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

#include "tabcompletionobject.h"
using namespace MathematiK;

#include <QStringList>
#include <QTimer>

#include "session.h"

class MathematiK::TabCompletionObjectPrivate
{
  public:
    QStringList completions;
    QString command;
    Session* session;
};

TabCompletionObject::TabCompletionObject(const QString& command, Session* session) :
                                                                                     d(new TabCompletionObjectPrivate)
{
    setParent(session);
    d->command=command;
    d->session=session;

    //start a delayed fetch
    QTimer::singleShot(0, this, SLOT(fetchCompletions()));
}

TabCompletionObject::~TabCompletionObject()
{
    delete d;
}

QString TabCompletionObject::command()
{
    return d->command;
}

Session* TabCompletionObject::session()
{
    return d->session;
}

QStringList TabCompletionObject::completions()
{
    return d->completions;
}

void TabCompletionObject::setCompletions(const QStringList& completions)
{
    d->completions=completions;
    foreach(const QString& comp, d->completions)
    {
        this->addItem(comp);
    }
}

#include "tabcompletionobject.moc"
