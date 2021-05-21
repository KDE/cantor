/*
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef _SAGEKEYWORDS_H
#define _SAGEKEYWORDS_H

#include <QStringList>

/**
  Class to store all Sage keywords (i.e. Python keywords)
  It is similar to MaximaKeywords or ScilabKeywords, but for Sage we only
  need to store actual keywords, as variables and functions can be fetched from
  the backend.
 */

class SageKeywords
{
private:
    SageKeywords() = default;
    ~SageKeywords() = default;

public:
    static SageKeywords* instance();

    const QStringList& keywords() const;
    const QStringList& functions() const;
    const QStringList& variables() const;

  private:
    void loadKeywords();

    QStringList m_keywords;
    QStringList m_functions;
    QStringList m_variables;
};

#endif /* _SAGEKEYWORDS_H */
