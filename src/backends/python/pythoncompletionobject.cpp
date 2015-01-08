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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#include <QDebug>

#include "pythoncompletionobject.h"

#include "pythonsession.h"
#include "pythonkeywords.h"

PythonCompletionObject::PythonCompletionObject(const QString& command, int index, PythonSession* session) : Cantor::CompletionObject(session)
{
    setLine(command, index);
}

PythonCompletionObject::~PythonCompletionObject()
{

}

void PythonCompletionObject::fetchCompletions()
{
    QStringList allCompletions;

    allCompletions << PythonKeywords::instance()->variables();
    allCompletions << PythonKeywords::instance()->functions();
    allCompletions << PythonKeywords::instance()->keywords();

    setCompletions(allCompletions);

    emit fetchingDone();
}

void PythonCompletionObject::fetchIdentifierType()
{
    // Scilab's typeof function could be used here, but as long as these lists
    // are used just looking up the name is easier.

    if (qBinaryFind(PythonKeywords::instance()->functions().begin(),
		    PythonKeywords::instance()->functions().end(), identifier())
	!= PythonKeywords::instance()->functions().end())
	emit fetchingTypeDone(FunctionType);
    else if (qBinaryFind(PythonKeywords::instance()->keywords().begin(),
			 PythonKeywords::instance()->keywords().end(), identifier())
	!= PythonKeywords::instance()->keywords().end())
	emit fetchingTypeDone(KeywordType);
    else
	emit fetchingTypeDone(VariableType);
}

bool PythonCompletionObject::mayIdentifierContain(const QChar& c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('%') || c == QLatin1Char('$') || c == QLatin1Char('.');
}

bool PythonCompletionObject::mayIdentifierBeginWith(const QChar& c) const
{
    return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('%') || c == QLatin1Char('$');
}
