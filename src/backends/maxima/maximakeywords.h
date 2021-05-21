/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
    MaximaKeywords() = default;
    ~MaximaKeywords() = default;
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
