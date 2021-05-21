/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _SCILABKEYWORDS_H
#define _SCILABKEYWORDS_H

#include <QStringList>

class ScilabKeywords
{
    public:
        static ScilabKeywords* instance();

        const QStringList& functions() const;
        const QStringList& keywords() const;
        const QStringList& variables() const;

    private:
        ScilabKeywords();
        ~ScilabKeywords() = default;
        QStringList m_functions;
        QStringList m_keywords;
        QStringList m_variables;
};
#endif /* _SCILABKEYWORDS_H */
