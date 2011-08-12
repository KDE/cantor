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

#include "qalculatesyntaxhelpobject.h"
#include "qalculatesession.h"

#include <KLocale>
#include <KDebug>

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

QalculateSyntaxHelpObject::QalculateSyntaxHelpObject(const QString& command, QalculateSession* session)
    : SyntaxHelpObject(command, session), m_answer(QString())
{
}

void QalculateSyntaxHelpObject::fetchInformation()
{
    std::string cmd = command().remove("help").simplified().toLatin1().data();
    kDebug() << "HELP CALLED FOR:" << QString(cmd.c_str());
    ExpressionItem *item = CALCULATOR->getActiveExpressionItem(cmd);

    if (!item) {
        m_answer = i18n("No function, variable or unit with specified name exist.");
        return;
    }

    switch(item->type()) {
    case TYPE_FUNCTION:
        MathFunction *f = (MathFunction*) item;
        QString title = i18n("Function: %1").arg(item->title().c_str());
        const ExpressionName *ename = &f->preferredName(false);
        int iargs = f->maxargs();
        if(iargs < 0) {
            iargs = f->minargs() + 1;
        }
        QString str,str2,syntax;
        str += ename->name.c_str();
        str += "(";
        if(iargs != 0) {
            Argument *arg;
            Argument default_arg;
            for(int i2 = 1; i2 <= iargs; i2++) {
                if(i2 > f->minargs()) {
                    str += "[";
                }
                if(i2 > 1) {
                    str += QString(CALCULATOR->getComma().c_str());
                    str += " ";
                }
                arg = f->getArgumentDefinition(i2);
                if(arg && !arg->name().empty()) {
                    str2 = arg->name().c_str();
                } else {
                    str2 = "argument";
                    str2 += " ";
                    str2 += QString::number(i2);
                }
                str += str2;
                if(i2 > f->minargs()) {
                    str += "]";
                }
            }
            if(f->maxargs() < 0) {
                str += CALCULATOR->getComma().c_str();
                str += " ...";
            }
            str += ")";
            syntax = QString("<p>%1</p>").arg(str);

            for(int i2 = 1; i2 <= iargs; i2++) {
                arg = f->getArgumentDefinition(i2);
                if(arg && !arg->name().empty()) {
                    str = arg->name().c_str();
                } else {
                    str = QString::number(i2);
                }
                str += ": ";
                if(arg) {
                    str2 = arg->printlong().c_str();
                } else {
                    str2 = default_arg.printlong().c_str();
                }
                if(i2 > f->minargs()) {
                    str2 += " (";
                    //optional argument, in description
                    str2 += "optional";
                    if(!f->getDefaultValue(i2).empty()) {
                        str2 += ", ";
                        //argument default, in description
                        str2 += "default: ";
                        str2 += f->getDefaultValue(i2).c_str();
                    }
                    str2 += ")";
                }
                str += str2;
            }
        }
        QString arguments = QString("<p>%1</p>").arg(str);

        QString desc = QString("<p>%1</p>").arg(item->description().c_str());

        m_answer = title + desc + syntax + arguments;
    }
}

QString QalculateSyntaxHelpObject::answer()
{
    fetchInformation();
    return m_answer;
}

