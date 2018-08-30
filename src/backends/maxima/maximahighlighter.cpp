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
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximahighlighter.h"
#include "maximakeywords.h"
#include "maximasession.h"
#include "maximavariablemodel.h"

MaximaHighlighter::MaximaHighlighter(QObject* parent, MaximaSession* session) : Cantor::DefaultHighlighter(parent)
{
    //addRule(QRegExp("\\b[A-Za-z0-9_]+(?=\\()"), functionFormat());

    //Code highlighting the different keywords
    addKeywords(MaximaKeywords::instance()->keywords());

    addRule(QLatin1String("FIXME"), commentFormat());
    addRule(QLatin1String("TODO"), commentFormat());

    addFunctions(MaximaKeywords::instance()->functions());
    addVariables(MaximaKeywords::instance()->variables());

    //addRule(QRegExp("\".*\""), stringFormat());
    //addRule(QRegExp("'.*'"), stringFormat());

    commentStartExpression = QRegExp(QLatin1String("/\\*"));
    commentEndExpression = QRegExp(QLatin1String("\\*/"));

    connect(session->variableModel(), SIGNAL(variablesAdded(QStringList)), this, SLOT(addUserVariables(QStringList)));
    connect(session->variableModel(), SIGNAL(variablesRemoved(QStringList)), this, SLOT(removeUserVariables(QStringList)));
    connect(session->variableModel(), SIGNAL(functionsAdded(QStringList)), this, SLOT(addUserFunctions(QStringList)));
    connect(session->variableModel(), SIGNAL(functionsRemoved(QStringList)), this, SLOT(removeUserFunctions(QStringList)));

    MaximaVariableModel* model=static_cast<MaximaVariableModel*>(session->variableModel());
    addUserVariables(model->variableNames());
    addUserFunctions(model->functionNames());
}

void MaximaHighlighter::highlightBlock(const QString& text)
{
    if (skipHighlighting(text))
        return;

    //Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    setCurrentBlockState(-1);

    int commentLevel = 0;
    bool inString = false;
    int startIndex = -1;

    if (previousBlockState() > 0) {
        commentLevel = previousBlockState();
        startIndex = 0;
    } else if (previousBlockState() < -1) {
        inString = true;
        startIndex = 0;
    }

    for (int i = 0; i < text.size(); ++i) {
        if (text[i] == QLatin1Char('\\')) {
            ++i; // skip the next character
        } else if (text[i] == QLatin1Char('"') && commentLevel == 0) {
            if (!inString)
                startIndex = i;
            else
                setFormat(startIndex, i - startIndex + 1, stringFormat());
            inString = !inString;
        } else if (text.mid(i,2) == QLatin1String("/*") && !inString) {
            if (commentLevel == 0)
                startIndex = i;
            ++commentLevel;
            ++i;
        } else if (text.mid(i,2) == QLatin1String("*/") && !inString) {
            if (commentLevel == 0) {
                setFormat(i, 2, errorFormat());
                // undo the --commentLevel below, so we stay at 0
                ++commentLevel;
            } else if (commentLevel == 1) {
                setFormat(startIndex, i - startIndex + 2, commentFormat());
            }
            ++i;
            --commentLevel;
        }
    }

    if (inString) {
        setCurrentBlockState(-2);
        setFormat(startIndex, text.size() - startIndex, stringFormat());
    } else if (commentLevel > 0) {
        setCurrentBlockState(commentLevel);
        setFormat(startIndex, text.size() - startIndex, commentFormat());
    }
}

void MaximaHighlighter::addUserVariables(const QStringList variables)
{
    addVariables(variables);
}

void MaximaHighlighter::removeUserVariables(const QStringList variables)
{
    for (const QString& var : variables)
        removeRule(var);
}

void MaximaHighlighter::addUserFunctions(const QStringList functions)
{
    //remove the trailing (x)
    for (const QString& func : functions)
    {
        int idx=func.lastIndexOf(QLatin1Char('('));
        addRule(func.left(idx), functionFormat());
    }
}

void MaximaHighlighter::removeUserFunctions(const QStringList functions)
{
    //remove the trailing (x)
    for (const QString& func : functions)
    {
        int idx=func.lastIndexOf(QLatin1Char('('));
        removeRule(func.left(idx));
    }
}
