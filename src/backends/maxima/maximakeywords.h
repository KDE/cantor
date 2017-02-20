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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMAKEYWORDS_H
#define _MAXIMAKEYWORDS_H

#include <QStringList>

/*
  Class storing a list of names, known to maxima
  used for syntax highlighting and tab completion
 */
class MaximaKeywords
{
  private:
    MaximaKeywords();
    ~MaximaKeywords();
  public:
    static MaximaKeywords* instance();

    const QStringList& functions() const;
    const QStringList& keywords() const;
    const QStringList& variables() const;

  private:
    void loadKeywords();

  private:
    QStringList m_functions;
    QStringList m_keywords;
    QStringList m_variables;
};
#endif /* _MAXIMAKEYWORDS_H */
