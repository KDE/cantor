/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _PYTHONKEYWORDS_H
#define _PYTHONKEYWORDS_H

#include <QStringList>

class PythonKeywords
{
  private:
    PythonKeywords();
    ~PythonKeywords() = default;
  public:
    static PythonKeywords* instance();

    const QStringList& functions() const;
    const QStringList& keywords() const;
    const QStringList& variables() const;

    void loadFromModule(const QString& module, const QStringList& keywords);

  private:
    void loadKeywords();

  private:
    QStringList m_functions;
    QStringList m_keywords;
    QStringList m_variables;
};
#endif /* _PYTHONKEYWORDS_H */
