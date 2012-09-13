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

#include "maximahighlighter.h"
#include "maximakeywords.h"
#include "maximasession.h"
#include "maximavariablemodel.h"

#include <QTextEdit>
#include <kdebug.h>

MaximaHighlighter::MaximaHighlighter(QObject* parent, MaximaSession* session) : Cantor::DefaultHighlighter(parent)
{
    //addRule(QRegExp("\\b[A-Za-z0-9_]+(?=\\()"), functionFormat());

    //Code highlighting the different keywords
    addKeywords(MaximaKeywords::instance()->keywords());

    addRule("FIXME", commentFormat());
    addRule("TODO", commentFormat());

    addFunctions(MaximaKeywords::instance()->functions());
    addVariables(MaximaKeywords::instance()->variables());

    addRule(QRegExp("\".*\""), stringFormat());
    addRule(QRegExp("'.*'"), stringFormat());

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

    connect(session->variableModel(), SIGNAL(variablesAdded(QStringList)), this, SLOT(addUserVariables(QStringList)));
    connect(session->variableModel(), SIGNAL(variablesRemoved(QStringList)), this, SLOT(removeUserVariables(QStringList)));
    connect(session->variableModel(), SIGNAL(functionsAdded(QStringList)), this, SLOT(addUserFunctions(QStringList)));
    connect(session->variableModel(), SIGNAL(functionsRemoved(QStringList)), this, SLOT(removeUserFunctions(QStringList)));
}

MaximaHighlighter::~MaximaHighlighter()
{
}

void MaximaHighlighter::highlightBlock(const QString& text)
{
    if (skipHighlighting(text))
        return;

    //Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {

        int endIndex = commentEndExpression.indexIn(text,  startIndex);
        int commentLength;
        if (endIndex == -1) {

            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                + commentEndExpression.matchedLength();
        }
        setFormat(startIndex,  commentLength,  commentFormat());
        startIndex = commentStartExpression.indexIn(text,  startIndex + commentLength);
    }
}

void MaximaHighlighter::addUserVariables(const QStringList variables)
{
    addVariables(variables);
}

void MaximaHighlighter::removeUserVariables(const QStringList variables)
{
    foreach(const QString& var, variables)
        removeRule(var);
}

void MaximaHighlighter::addUserFunctions(const QStringList functions)
{
    //remove the trailing (x)
    foreach(const QString& func, functions)
    {
        int idx=func.lastIndexOf('(');
        addRule(func.left(idx), functionFormat());
    }
}

void MaximaHighlighter::removeUserFunctions(const QStringList functions)
{
    //remove the trailing (x)
    foreach(const QString& func, functions)
    {
        int idx=func.lastIndexOf('(');
        removeRule(func.left(idx));
    }

}

