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
#include <QDebug>

#include "session.h"

class Cantor::CompletionObjectPrivate
{
  public:
    QStringList completions;
    QString line;
    QString command;
    QString identifier;
    QString completion;
    int position;
    Session* session;
    bool parenCompletion;
};

CompletionObject::CompletionObject(Session* session) :
    d(new CompletionObjectPrivate)
{
    setParent(session);
    d->position = -1;
    d->session=session;

    connect(this, &CompletionObject::fetchingDone, this, &CompletionObject::findCompletion);
    connect(this, &CompletionObject::fetchingTypeDone, this, &CompletionObject::completeLineWithType);

    setCompletionMode(KCompletion::CompletionShell);
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

QString CompletionObject::identifier() const
{
    return d->identifier;
}

QString CompletionObject::completion() const
{
    return d->completion;
}

void CompletionObject::setLine(const QString& line, int index)
{
    d->parenCompletion = false;
    d->line = line;
    if (index < 0)
	index = line.length();
    if (index > 1 && line[index-1] == QLatin1Char('(')) {
	--index;                   // move before the parenthesis
	d->parenCompletion = true; // but remember it was there
    }
    int cmd_index = locateIdentifier(line, index-1);
    if (cmd_index < 0)
	cmd_index = index;
    d->position=cmd_index;
    d->command=line.mid(cmd_index, index-cmd_index);

    //start a delayed fetch
    QTimer::singleShot(0, this, SLOT(fetchCompletions()));
}

void CompletionObject::updateLine(const QString& line, int index)
{
    d->line = line;
    if (index < 0)
	index = line.length();
    int cmd_index = locateIdentifier(line, index-1);
    if (cmd_index < 0)
	cmd_index = index;
    d->command=line.mid(cmd_index, index-cmd_index);

    // start a delayed fetch
    // For some backends this is a lot of unnecessary work...
    QTimer::singleShot(0, this, SLOT(fetchCompletions()));
}

void CompletionObject::completeLine(const QString& comp, CompletionObject::LineCompletionMode mode)
{
    d->identifier = comp;
    if (comp.isEmpty()) {
	int index = d->position + d->command.length();
	emit lineDone(d->line, index);
    } else if (mode == PreliminaryCompletion) {
	completeUnknownLine();
    } else /* mode == FinalCompletion */ {
	QTimer::singleShot(0, this, SLOT(fetchIdentifierType()));
    }
}

void CompletionObject::fetchIdentifierType()
{
    emit fetchingTypeDone(UnknownType);
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
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_');
}

bool CompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_');
}

void CompletionObject::findCompletion()
{
    if (d->parenCompletion) {
	disconnect(this, SIGNAL(fetchingTypeDone(IdentifierType)), 0, 0);
	connect(this, &CompletionObject::fetchingTypeDone, this, &CompletionObject::handleParenCompletionWithType);
	d->identifier = d->command;
	fetchIdentifierType();
	return;
    }
    d->completion = makeCompletion(command());
    emit done();
}

void CompletionObject::handleParenCompletionWithType(IdentifierType type)
{
    disconnect(this, SIGNAL(fetchingTypeDone(IdentifierType)), 0, 0);
    connect(this, &CompletionObject::fetchingTypeDone, this, &CompletionObject::completeLineWithType);

    if (type == FunctionWithArguments || type == FunctionWithoutArguments) {
	d->completion = d->command;
	emit done();
    }
}

void CompletionObject::completeLineWithType(IdentifierType type)
{
    switch(type) {
    case VariableType:
	completeVariableLine();
	break;
    case FunctionWithArguments:
    case FunctionWithoutArguments:
	completeFunctionLine(type);
	break;
    case KeywordType:
	completeKeywordLine();
	break;
    case UnknownType:
	completeUnknownLine();
	break;
    }
}

void CompletionObject::completeFunctionLine(IdentifierType type)
{
    QString newline;
    int newindex;

    QString func = d->identifier;
    int after_command =  d->position + d->command.length();
    QString part1 = d->line.left(d->position) + func;
    int index = d->position + func.length() + 1;
    if (after_command < d->line.length() && d->line.at(after_command) == QLatin1Char('(')) {
	QString part2 = d->line.mid(after_command+1);
	int i;
	// search for next non-space position
	for (i = after_command+1;
	     i < d->line.length() && d->line.at(i).isSpace();
	     ++i) {}
	if (type == FunctionWithArguments) {
	    if (i < d->line.length()) {
		newline = part1+QLatin1Char('(')+part2;
		newindex = index;
	    } else {
		newline = part1+QLatin1String("()")+part2;
		newindex = index;
	    }
	} else /*type == FunctionWithoutArguments*/ {
	    if (i < d->line.length() && d->line.at(i) == QLatin1Char(')')) {
		newline = part1+QLatin1Char('(')+part2;
		newindex = index+i-after_command;
	    } else {
		newline = part1+QLatin1String("()")+part2;
		newindex = index+1;
	    }
	}
    } else {
	QString part2 = d->line.mid(after_command);
	if (type == FunctionWithArguments) {
	    newline = part1+QLatin1String("()")+part2;
	    newindex = index;
	} else /*type == FunctionWithoutArguments*/ {
	    newline = part1+QLatin1String("()")+part2;
	    newindex = index+1;
	}
    }
    emit lineDone(newline, newindex);
}

void CompletionObject::completeKeywordLine()
{
    QString keyword = d->identifier;
    int after_command = d->position + d->command.length();
    int newindex = d->position + keyword.length() + 1;
    QString part1 = d->line.left(d->position) + keyword;
    QString part2 = d->line.mid(after_command);
    if (after_command < d->line.length() && d->line.at(after_command) == QLatin1Char(' '))
	emit lineDone(part1+part2, newindex);
    else
	emit lineDone(part1+QLatin1Char(' ')+part2, newindex);
}

void CompletionObject::completeVariableLine()
{
    QString var = d->identifier;
    int after_command = d->position + d->command.length();
    QString newline = d->line.left(d->position) + var + d->line.mid(after_command);
    int newindex = d->position + var.length();
    emit lineDone(newline, newindex);
}

void CompletionObject::completeUnknownLine()
{
    // identifiers of unknown type are completed like variables
    completeVariableLine();
}


