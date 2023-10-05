/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#include "luacompletionobject.h"

#include <QStringList>

#include "luasession.h"
#include "luahelper.h"
#include "luakeywords.h"

LuaCompletionObject::LuaCompletionObject(const QString& command, int index, LuaSession* session)
    : Cantor::CompletionObject(session)
{
    if (session->status() != Cantor::Session::Disable)
        m_L = session->getState();
    else
        m_L = nullptr;
    setLine(command, index);
}

void LuaCompletionObject::fetchCompletions()
{
    if (session()->status() != Cantor::Session::Done)
    {
        QStringList allCompletions;

        allCompletions << LuaKeywords::instance()->keywords();
        allCompletions << LuaKeywords::instance()->functions();
        allCompletions << LuaKeywords::instance()->variables();

        setCompletions(allCompletions);
        Q_EMIT fetchingDone();
    }
    else
    {
        QString name = command();
        int idx = name.lastIndexOf(QLatin1String("="));

        // gets "table.next" from the expression "varname =   table.next"
        if(idx >= 0)
            name = name.mid(idx+1).trimmed();

        setCompletions( luahelper_completion(m_L, name) );
        Q_EMIT fetchingDone();
    }
}

bool LuaCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char(':');
}

bool LuaCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_');
}
