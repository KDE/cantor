/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _LUAKEYWORDS_H
#define _LUAKEYWORDS_H

#include <QStringList>

class LuaKeywords
{
    public:
        static LuaKeywords* instance();

        const QStringList& functions() const;
        const QStringList& keywords() const;
        const QStringList& variables() const;

    private:
        LuaKeywords();
        ~LuaKeywords() = default;
        QStringList m_functions;
        QStringList m_keywords;
        QStringList m_variables;
};
#endif /* _LUAKEYWORDS_H */
