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

#include "maximacompletionobject.h"

#include "maximasession.h"
#include "maximakeywords.h"

MaximaCompletionObject::MaximaCompletionObject(const QString& command, MaximaSession* session) : Cantor::CompletionObject(command, session)
{
    //Only use the completion for the last command part between end and opening bracket or ; or space
    QString cmd=command;
    int brIndex=cmd.lastIndexOf('(')+1;
    int semIndex=cmd.lastIndexOf(';')+1;
    int spaceIndex=cmd.lastIndexOf(' ')+1;

    cmd=cmd.mid(qMax(brIndex, qMax(semIndex, spaceIndex)));

    setCommand(cmd);
}

MaximaCompletionObject::~MaximaCompletionObject()
{

}

void MaximaCompletionObject::fetchCompletions()
{
    QStringList allCompletions;
    allCompletions<<MaximaKeywords::instance()->variables();
    allCompletions<<MaximaKeywords::instance()->functions();
    allCompletions<<MaximaKeywords::instance()->keywords();

    setCompletions(allCompletions);

    emit done();
}
