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
    Copyright (C) 2017 Rishabh Gupta <rishabh9511@gmail.com>
 */

#include <QString>
#include <QStringList>

/**
 * RKeywords is used to maintain a list of all the built-ins available in R language
 * Built-ins are: keywords(if,else,etc), functions(min(), max(),etc), operators(+,-, etc)
 * List of keywords and operators can be found at https://cran.r-project.org/doc/manuals/R-lang.pdf
 * Other than built-ins, it's also used to maintain a list of specials(TODO,WARNING etc)
 *
 */

class RKeywords
{
private:
    RKeywords();
    static RKeywords* _instance;
    QString m_builtIns;
    QStringList m_keywordsList;
    QStringList m_functionsList;
    QStringList m_specialsList;
    QStringList m_operatorsList;

    bool isKeyword(QString& key);
    bool isOperator(QString& key);


public:

    static RKeywords* getInstance();
    void setupBuiltIns(QString& builtIns);
    void parseBuiltIns();
    QStringList& getKeywords();
    QStringList& getFunctions();
    QStringList& getSpecials();
    QStringList& getOperators();

};
