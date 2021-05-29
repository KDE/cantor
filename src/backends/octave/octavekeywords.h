/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _OCTAVEKEYWORDS_H
#define _OCTAVEKEYWORDS_H

#include <QStringList>

class OctaveKeywords
{
    public:
        static OctaveKeywords* instance();

        const QStringList& functions() const;
        const QStringList& keywords() const;

    private:
        OctaveKeywords();
        ~OctaveKeywords() = default;
        QStringList m_functions;
        QStringList m_keywords;
};
#endif /* _OCTAVEKEYWORDS_H */
