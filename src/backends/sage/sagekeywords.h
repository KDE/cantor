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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */
#ifndef _SAGEKEYWORDS_H
#define _SAGEKEYWORDS_H

#include <QStringList>

/*
  Class to store all Sage keywords (i.e. Python keywords)
  It is similiar to MaximaKeywords or ScilabKeywords, but for Sage we only
  need to store actual keywords, as variables and functions can be fetched from
  the backend.
 */

class SageKeywords
{
private:
    SageKeywords();
    ~SageKeywords();
public:
    static SageKeywords* instance();

    const QStringList& functions() const;
    const QStringList& keywords() const;
    const QStringList& variables() const;

  private:
    void loadFromFile();

    QStringList m_keywords;
};

#endif /* _SAGEKEYWORDS_H */
