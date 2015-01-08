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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#ifndef _PYTHONKEYWORDS_H
#define _PYTHONKEYWORDS_H

#include <QStringList>

class PythonKeywords
{
  private:
    PythonKeywords();
    ~PythonKeywords();
  public:
    static PythonKeywords* instance();

    const QStringList& functions() const;
    const QStringList& keywords() const;
    const QStringList& variables() const;

    void loadFromModule(const QString& module, const QStringList& keywords);
    void addVariable(const QString& variable);

  private:
    void loadFromFile();

  private:
    QStringList m_functions;
    QStringList m_keywords;
    QStringList m_variables;
};
#endif /* _PYTHONKEYWORDS_H */
