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
    Copyright (C) 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
 */

#include "luacompletionobject.h"

#include <QStringList>

#include "luasession.h"
#include "luahelper.h"

LuaCompletionObject::LuaCompletionObject(const QString& command, int index, LuaSession* session)
    : Cantor::CompletionObject(session), m_L(session->getState())
{
    setLine(command, index);
}

void LuaCompletionObject::fetchCompletions()
{
    QString name = command();
    int idx = name.lastIndexOf(QLatin1String("="));

    // gets "table.next" from the expression "varname =   table.next"
    if(idx >= 0)
        name = name.mid(idx+1).trimmed();

    setCompletions( luahelper_completion(m_L, name) );
    emit fetchingDone();
}

bool LuaCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char(':');
}

bool LuaCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_');
}
