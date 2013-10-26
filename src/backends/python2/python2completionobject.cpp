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

#include <kdebug.h>

#include "python2completionobject.h"

#include "python2session.h"
#include "python2keywords.h"

Python2CompletionObject::Python2CompletionObject(const QString& command, int index, Python2Session* session) : Cantor::CompletionObject(session)
{
    setLine(command, index);
}

Python2CompletionObject::~Python2CompletionObject()
{

}

void Python2CompletionObject::fetchCompletions()
{
    QStringList allCompletions;

    allCompletions << Python2Keywords::instance()->variables();
    allCompletions << Python2Keywords::instance()->functions();
    allCompletions << Python2Keywords::instance()->keywords();

    setCompletions(allCompletions);

    emit fetchingDone();
}

void Python2CompletionObject::fetchIdentifierType()
{
    // Scilab's typeof function could be used here, but as long as these lists
    // are used just looking up the name is easier.

    if (qBinaryFind(Python2Keywords::instance()->functions().begin(),
		    Python2Keywords::instance()->functions().end(), identifier())
	!= Python2Keywords::instance()->functions().end())
	emit fetchingTypeDone(FunctionType);
    else if (qBinaryFind(Python2Keywords::instance()->keywords().begin(),
			 Python2Keywords::instance()->keywords().end(), identifier())
	!= Python2Keywords::instance()->keywords().end())
	emit fetchingTypeDone(KeywordType);
    else
	emit fetchingTypeDone(VariableType);
}

bool Python2CompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == '_' || c == '%' || c == '$' || c == '.';
}

bool Python2CompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == '_' || c == '%' || c == '$';
}
