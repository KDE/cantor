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
    QString context;
    QString command;
    int position;
    Session* session;
};

CompletionObject::CompletionObject(const QString& command, int index, Session* session) :
    d(new CompletionObjectPrivate)
{
    setParent(session);
    d->context=command;
    d->session=session;
    if (index < 0)
	index = command.length();
    int cmd_index = locateIdentifier(command, index-1);
    d->position=cmd_index;
    if (cmd_index < 0)
	d->command="";
    else
	d->command=command.mid(cmd_index, index-cmd_index);

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
    this->setItems(completions);
    /*foreach(const QString& comp, d->completions)
    {
        this->addItem(comp);
    }
    */
}

void CompletionObject::setCommand(const QString& cmd)
{
    d->command=cmd;
}

int CompletionObject::locateIdentifier(const QString& cmd, int index) const
{
    if (index < 0)
	return -1;

    int i;
    for (i=index; i>=0 && mayIdentifierContain(cmd[i]); --i) {
    }
    
    if (!mayIdentifierBeginWith(cmd[i+1]))
	return -1;
    return i+1;
}

bool CompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetterOrNumber() or c == '_';
}

bool CompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() or c == '_';
}


#include "completionobject.moc"
