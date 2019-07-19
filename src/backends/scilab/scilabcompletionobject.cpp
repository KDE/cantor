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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "scilabcompletionobject.h"

#include <QDebug>

#include "scilabsession.h"
#include "scilabkeywords.h"

ScilabCompletionObject::ScilabCompletionObject(const QString& command, int index, ScilabSession* session) : Cantor::CompletionObject(session)
{
    setLine(command, index);
}

void ScilabCompletionObject::fetchCompletions()
{
    // A more elegant approach would be to use Scilab's completion() function,
    // similarly to how fetching is done in OctaveCompletionObject.
    // Unfortunately its interactive behavior is not handled well by cantor.
    QStringList allCompletions;

    allCompletions << ScilabKeywords::instance()->variables();
    allCompletions << ScilabKeywords::instance()->functions();
    allCompletions << ScilabKeywords::instance()->keywords();

    setCompletions(allCompletions);

    emit fetchingDone();
}

void ScilabCompletionObject::fetchIdentifierType()
{
    // Scilab's typeof function could be used here, but as long as these lists
    // are used just looking up the name is easier.

    if (std::binary_search(ScilabKeywords::instance()->functions().begin(), ScilabKeywords::instance()->functions().end(),
            identifier()))
        emit fetchingTypeDone(FunctionType);
    else if (std::binary_search(ScilabKeywords::instance()->keywords().begin(),ScilabKeywords::instance()->keywords().end(),
            identifier()))
        emit fetchingTypeDone(KeywordType);
    else
        emit fetchingTypeDone(VariableType);
}

bool ScilabCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('%') || c == QLatin1Char('$');
}

bool ScilabCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('%') || c == QLatin1Char('$');
}
