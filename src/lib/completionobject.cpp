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
#include <KDebug>

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
    if (cmd_index < 0)
	cmd_index = index;
    d->position=cmd_index;
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

QPair<QString, int> CompletionObject::completeLine(const QString& comp, CompletionObject::LineCompletionMode mode)
{
    Q_UNUSED(mode);
    if (comp.isEmpty()) {
	int index = d->position + d->command.length();
	return QPair<QString, int>(d->context, index);
    } else {
	QString newline = d->context.left(d->position) + comp + 
	    d->context.mid(d->position + d->command.length());
	int newindex = d->position + comp.length();
	return QPair<QString, int>(newline, newindex);
    }
}

void CompletionObject::setCompletions(const QStringList& completions)
{
    d->completions=completions;
    this->setItems(completions);
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
    for (i=index; i>=0 && mayIdentifierContain(cmd[i]); --i) 
	{}
    
    if (i==index || !mayIdentifierBeginWith(cmd[i+1]))
	return -1;
    return i+1;
}

bool CompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == '_';
}

bool CompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == '_';
}

QPair<QString, int> CompletionObject::completeFunctionLine(const QString& func, FunctionType type) const
{
    int after_command =  d->position + d->command.length();
    QString part1 = d->context.left(d->position) + func;
    int index = d->position + func.length() + 1;
    if (after_command < d->context.length() && d->context.at(after_command) == '(') {
	QString part2 = d->context.mid(after_command+1);
	int i;
	// search for next non-space position
	for (i = after_command+1; 
	     i < d->context.length() && d->context.at(i).isSpace(); 
	     ++i) {}
	if (type == HasArguments) {
	    if (i < d->context.length()) {
		return QPair<QString, int>(part1+'('+part2, index);
	    } else {
		return QPair<QString, int>(part1+"()"+part2, index);
	    }
	} else /*type == HasNoArguments*/ {
	    if (i < d->context.length() && d->context.at(i) == ')') {
		return QPair<QString, int>(part1+'('+part2, index+i-after_command);
	    } else {
		return QPair<QString, int>(part1+"()"+part2, index+1);
	    }
	}
    } else {
	QString part2 = d->context.mid(after_command);
	if (type == HasArguments)
	    return QPair<QString, int>(part1+"()"+part2, index);
	else /*type == HasNoArguments*/
	    return QPair<QString, int>(part1+"()"+part2, index+1);
    }
}

QPair<QString, int> CompletionObject::completeKeywordLine(const QString& keyword) const
{
    int after_command = d->position + d->command.length();
    int newindex = d->position + keyword.length() + 1;
    QString part1 = d->context.left(d->position) + keyword;
    QString part2 = d->context.mid(after_command);
    if (after_command < d->context.length() && d->context.at(after_command) == ' ')
	return QPair<QString, int>(part1+part2, newindex);
    else
	return QPair<QString, int>(part1+' '+part2, newindex);
}

QPair<QString, int> CompletionObject::completeVariableLine(const QString& var) const
{
    int after_command = d->position + d->command.length();
    QString newline = d->context.left(d->position) + var + d->context.mid(after_command);
    int newindex = d->position + var.length();
    return QPair<QString, int>(newline, newindex);
}

#include "completionobject.moc"
