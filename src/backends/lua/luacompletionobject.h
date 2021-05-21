/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#ifndef _LUACOMPLETIONOBJECT_H
#define _LUACOMPLETIONOBJECT_H

#include "completionobject.h"

class LuaSession;
class QString;
struct lua_State;

class LuaCompletionObject : public Cantor::CompletionObject
{
public:
    LuaCompletionObject( const QString& command, int index, LuaSession* session);
    ~LuaCompletionObject() override = default;

protected Q_SLOTS:
    void fetchCompletions() override;

protected:
    bool mayIdentifierContain(QChar c)   const override;
    bool mayIdentifierBeginWith(QChar c) const override;

private:
    lua_State* m_L;
};

#endif /* _LUACOMPLETIONOBJECT_H */
