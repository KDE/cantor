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

#include <kdebug.h>

#include "maximacompletionobject.h"

#include "maximasession.h"
#include "maximakeywords.h"

MaximaCompletionObject::MaximaCompletionObject(const QString& command, int index,MaximaSession* session) : Cantor::CompletionObject(command, index, session)
{
    kDebug() << "MaximaCompletionObject construtor";
}

MaximaCompletionObject::~MaximaCompletionObject()
{

}

Cantor::CompletionObject::IdentifierType MaximaCompletionObject::identifierType(const QString& identifier) const
{
    if (qBinaryFind(MaximaKeywords::instance()->functions().begin(),
		    MaximaKeywords::instance()->functions().end(), identifier)
	!= MaximaKeywords::instance()->functions().end())
	return FunctionIdentifier;
    else if (qBinaryFind(MaximaKeywords::instance()->keywords().begin(),
			 MaximaKeywords::instance()->keywords().end(), identifier)
	!= MaximaKeywords::instance()->keywords().end())
	return KeywordIdentifier;
    else
	return VariableIdentifier;
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

bool MaximaCompletionObject::mayIdentifierContain(QChar c) const
{
    kWarning() << '%';
    return c.isLetter() || c.isDigit() || c == '_' || c == '%';
}

bool MaximaCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    kWarning() << '%';
    return c.isLetter() || c == '_' || c == '%';
}
