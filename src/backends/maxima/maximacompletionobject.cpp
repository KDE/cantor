/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "maximacompletionobject.h"

#include <QDebug>

#include "maximasession.h"
#include "maximakeywords.h"
#include "maximavariablemodel.h"

MaximaCompletionObject::MaximaCompletionObject(const QString& command, int index,MaximaSession* session) : Cantor::CompletionObject(session)
{
    qDebug() << "MaximaCompletionObject constructor";
    setLine(command, index);
}

void MaximaCompletionObject::fetchIdentifierType()
{
    QStringList userVariableNames=session()->variableModel()->variableNames();
    QStringList userFunctionNames=session()->variableModel()->functions();
    if (std::binary_search(userVariableNames.begin(), userVariableNames.end(), identifier()))
        emit fetchingTypeDone(VariableType);
    else if (std::binary_search(userFunctionNames.begin(), userFunctionNames.end(), identifier()))
        emit fetchingTypeDone(FunctionType);
    else if (std::binary_search(MaximaKeywords::instance()->functions().begin(),
            MaximaKeywords::instance()->functions().end(), identifier()))
        emit fetchingTypeDone(FunctionType);
    else if (std::binary_search(MaximaKeywords::instance()->keywords().begin(),
            MaximaKeywords::instance()->keywords().end(), identifier()))
        emit fetchingTypeDone(KeywordType);
    else
        emit fetchingTypeDone(VariableType);
}

void MaximaCompletionObject::fetchCompletions()
{
    QStringList allCompletions;
    allCompletions<<MaximaKeywords::instance()->variables();
    allCompletions<<MaximaKeywords::instance()->functions();
    allCompletions<<MaximaKeywords::instance()->keywords();
    allCompletions<<session()->variableModel()->variableNames();
    allCompletions<<session()->variableModel()->functions();

    const QString prefix = command();
    QStringList prefixCompletion;
    for (const QString& str : allCompletions)
        if (str.startsWith(prefix))
            prefixCompletion << str;

    setCompletions(prefixCompletion);

    emit fetchingDone();
}

bool MaximaCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') || c == QLatin1Char('%');
}

bool MaximaCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('%');
}
