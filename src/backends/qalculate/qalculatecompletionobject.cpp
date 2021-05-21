/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qalculatecompletionobject.h"

#include <QStringList>

#include <libqalculate/Calculator.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include "qalculatesession.h"

QalculateCompletionObject::QalculateCompletionObject(const QString& command, int index, QalculateSession* session)
    : Cantor::CompletionObject(session)
{
    setLine(command, index);
}

void QalculateCompletionObject::fetchIdentifierType()
{
    Variable* var = CALCULATOR->getVariable(identifier().toLatin1().data());
    if (var) {
        emit fetchingTypeDone(VariableType);
        return;
    }
    MathFunction* func = CALCULATOR->getFunction(identifier().toLatin1().data());
    if (!func) // can this happen?
        emit fetchingTypeDone(UnknownType);
    else if (func->args() == 0)
        emit fetchingTypeDone(FunctionWithoutArguments);
    else
        emit fetchingTypeDone(FunctionWithArguments);
}

int QalculateCompletionObject::locateIdentifier(const QString& cmd, int index) const
{
    if (index < 0)
	return -1;

    int i;
    int start_index = -1;
    for (i=index; i>=0 && mayIdentifierContain(cmd[i]); --i) {
	if (mayIdentifierBeginWith(cmd[i]))
	    start_index = i;
    }

    return start_index;
}


void QalculateCompletionObject::fetchCompletions()
{
    QStringList comp;
    // Matching Qt::CaseInsensitive here does not help, because a) Qalculate
    // does distinguish cases, and b) KCompletion::makeCompletion matches
    // case sensitive.
    foreach ( Unit* item, CALCULATOR->units ) {
        //TODO: this is fugily...
        QString str(QLatin1String(item->name(true).c_str()));
        if ( str.startsWith(command(), Qt::CaseSensitive) ) {
            comp << str;
        }
    QString str2(QLatin1String(item->singular().c_str()));
    if (str2.startsWith(command(), Qt::CaseSensitive) ) {
        comp << str2;
    }
    // Also include the plural form for completion?
    //QString str3(item->plural().c_str());
    //if (str3.startsWith(command(), Qt::CaseSensitive) ) {
    //    comp << str3;
    //}
    }
    foreach ( ExpressionItem* item, CALCULATOR->variables ) {
        //TODO: this is fugly...
        QString str(QLatin1String(item->name(true).c_str()));
        if ( str.startsWith(command(), Qt::CaseSensitive) ) {
            comp << str;
        }
    }
    foreach ( ExpressionItem* item, CALCULATOR->functions ) {
        //TODO: this is fugly...
        QString str(QLatin1String(item->name(true).c_str()));
        if ( str.startsWith(command(), Qt::CaseSensitive) ) {
            comp << str;
        }
    }

    setCompletions(comp);
    emit fetchingDone();
}
