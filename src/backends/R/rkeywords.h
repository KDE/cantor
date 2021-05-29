/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _RKEYWORDS_H
#define _RKEYWORDS_H

#include <QStringList>

class RKeywords
{
    public:
        static RKeywords* instance();

        const QStringList& keywords() const;

    private:
        RKeywords();
        ~RKeywords() = default;
        QStringList m_keywords;
};
#endif /* _RKEYWORDS_H */
