/*************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "qalculatecompletionobject.h"

#include <QStringList>

#include <libqalculate/Calculator.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include "qalculatesession.h"

#include <KDebug>

QalculateCompletionObject::QalculateCompletionObject(const QString& command, int index, QalculateSession* session)
    : Cantor::CompletionObject(command, index, session)
{
}

QalculateCompletionObject::~QalculateCompletionObject()
{
}

QPair<QString, int> QalculateCompletionObject::completeLine(const QString& comp, LineCompletionMode mode)
{
    // TODO: This test is already in CompletionObject::completeLine;
    // it should not be necessary in every backend.
    //if (comp.isEmpty()) {
    //	int index = d->position + d->command.length();
    //	return QPair<QString, int>(d->context, index);
    //}
    if (mode == PreliminaryCompletion)
	return this->CompletionObject::completeLine(comp, mode);
    Variable* var = CALCULATOR->getVariable(comp.toLatin1().data());
    if (var)
	return completeVariableLine(comp);
    MathFunction* func = CALCULATOR->getFunction(comp.toLatin1().data());
    if (!func) // this should not happen...
	return this->CompletionObject::completeLine(comp, mode);
    else if (func->args() == 0)
	return completeFunctionLine(comp, HasNoArguments);
    else 
	return completeFunctionLine(comp, HasArguments);
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
    foreach ( ExpressionItem* item, CALCULATOR->variables ) {
        //TODO: this is fugly...
        QString str(item->name(true).c_str());
        if ( str.startsWith(command(), Qt::CaseInsensitive) ) {
            comp << str;
        }
    }
    foreach ( ExpressionItem* item, CALCULATOR->functions ) {
        //TODO: this is fugly...
        QString str(item->name(true).c_str());
        if ( str.startsWith(command(), Qt::CaseInsensitive) ) {
            comp << str;
        }
    }

    setCompletions(comp);
    emit done();
}
