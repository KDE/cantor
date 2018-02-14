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


#include "rkeywords.h"
#include <QDebug>
#include <QFile>
#include <QRegExp>

RKeywords* RKeywords::_instance = 0;

RKeywords::RKeywords()
{
    m_keywordsList << QLatin1String("if") << QLatin1String("else") << QLatin1String("switch")
    << QLatin1String("while") << QLatin1String("for") << QLatin1String("repeat") << QLatin1String("function")
    << QLatin1String("in") << QLatin1String("next") << QLatin1String("break") << QLatin1String("TRUE")
    << QLatin1String("FALSE") << QLatin1String("NULL") << QLatin1String("NA") << QLatin1String("NA_integer_")
    << QLatin1String("NA_real_") << QLatin1String("NA_complex_") << QLatin1String("NA_character_")
    << QLatin1String("Inf") << QLatin1String("NaN");

    m_operatorsList << QLatin1String("+") << QLatin1String("-") << QLatin1String("*") << QLatin1String("/")
    << QLatin1String("%%") << QLatin1String("^") << QLatin1String(">") << QLatin1String("<")
    << QLatin1String(">=") << QLatin1String("<=") << QLatin1String("==") << QLatin1String("!=")
    << QLatin1String("!") << QLatin1String("&") << QLatin1String("|") << QLatin1String("~")
    << QLatin1String("<-") << QLatin1String("->") << QLatin1String("$") << QLatin1String("$")
    << QLatin1String(":");

    m_specialsList << QLatin1String("BUG") << QLatin1String("TODO") << QLatin1String("FIXME") << QLatin1String("NB") << QLatin1String("WARNING") << QLatin1String("ERROR");


}


RKeywords*  RKeywords::getInstance()
{
    if(!_instance) {
        _instance = new RKeywords();
    }

    return _instance;
}

void RKeywords::setupBuiltIns(QString& builtIns)
{
    /**
     * builtIns is a string containg R's built in functions, keywords and operators.
     * This is generated using the command 'apropos('')'.
     * @see RSession::loadBuiltIns
     */

    m_builtIns = builtIns;
    parseBuiltIns();

}


QStringList& RKeywords::getKeywords()
{
        return m_keywordsList;
}

QStringList& RKeywords::getFunctions()
{
    return m_functionsList;
}

QStringList& RKeywords::getSpecials()
{
    return m_specialsList;
}

QStringList& RKeywords::getOperators()
{
    return m_operatorsList;
}


void RKeywords::parseBuiltIns()
{
    /**
     * m_builtIns is a string containg R's built in functions, keywords and operators
     * we need to parse the string to separate out functions. we already have keywords and operators
     * Algo: check if the current string is a part of m_keywordsList/m_operatorsList, if no, push the current
     *       word to m_functionsList
     */

    // remove white space, prompt, command(apropos)
    m_builtIns.replace(QLatin1String(" "), QLatin1String(""));
    m_builtIns.replace(QLatin1String("apropos('')"), QLatin1String(""));
    m_builtIns.replace(QLatin1String(">"), QLatin1String(""));
    m_builtIns.replace(QRegExp(QLatin1String("\"")),QLatin1String(""));
    m_builtIns.replace(QRegExp(QLatin1String("\\[[0-9]+\\]")), QLatin1String(""));

    QStringList builtInsList = m_builtIns.split(QLatin1String("\n"));


    for(QString& key: builtInsList) {
        if(!key.isEmpty()) {

            if(!isKeyword(key) && !isOperator(key)) {
                // push back to functions list
                m_functionsList << key;
            }
        }

    }
    qDebug() << "size of functions list  " << m_functionsList.size() << endl;

}

bool RKeywords::isKeyword(QString& key)
{
    return m_keywordsList.contains(key);
}

bool RKeywords::isOperator(QString& key)
{
    return m_operatorsList.contains(key);
}


