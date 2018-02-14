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
    Copyright (C) 2010 Oleksiy Protas <elfy.ua@gmail.com>
 */

#include "rhighlighter.h"
#include "rkeywords.h"
#include <QTextEdit>
#include <QDebug>

RHighlighter::RHighlighter(QObject* parent) : Cantor::DefaultHighlighter(parent)
{
    RKeywords* instance = RKeywords::getInstance();

    addKeywords(instance->getKeywords());
    addFunctions(instance->getFunctions());
    addRules(instance->getOperators(), operatorFormat());
    addRules(instance->getSpecials(), commentFormat());

}

RHighlighter::~RHighlighter()
{
}

void RHighlighter::updateHighlighter()
{
    qDebug () << RKeywords::getInstance()->getFunctions().size() << endl;
    addFunctions(RKeywords::getInstance()->getFunctions());

}
