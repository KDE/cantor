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
    Copyright (C) 2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximavariablemodel.h"

#include "maximasession.h"
#include "maximaexpression.h"
#include "textresult.h"
#include "latexresult.h"

#include <QDebug>
#include <KLocalizedString>

//command used to inspect a maxima variable. %1 is the name of that variable
const QString MaximaVariableModel::inspectCommand=QLatin1String(":lisp($disp $%1)");
const QString MaximaVariableModel::variableInspectCommand=QLatin1String(":lisp(cantor-inspect $%1)");

MaximaVariableModel::MaximaVariableModel( MaximaSession* session) : Cantor::DefaultVariableModel(session)
{

}

void MaximaVariableModel::clear()
{
    emit functionsRemoved(functionNames());
    emit variablesRemoved(variableNames());
    m_functions.clear();
    m_variables.clear();
    DefaultVariableModel::clearVariables();
}

void MaximaVariableModel::checkForNewVariables()
{
    qDebug()<<"checking for new variables";
    const QString& cmd=variableInspectCommand.arg(QLatin1String("values"));
    Cantor::Expression* expr=session()->evaluateExpression(cmd, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
    connect(expr, &Cantor::Expression::statusChanged, this, &MaximaVariableModel::parseNewVariables);
}

void MaximaVariableModel::checkForNewFunctions()
{
    qDebug()<<"checking for new functions";
    const QString& cmd=inspectCommand.arg(QLatin1String("functions"));
    Cantor::Expression* expr=session()->evaluateExpression(cmd, Cantor::Expression::FinishingBehavior::DoNotDelete, true);

    connect(expr, &Cantor::Expression::statusChanged, this, &MaximaVariableModel::parseNewFunctions);
}

QList<Cantor::DefaultVariableModel::Variable> parse(MaximaExpression* expr)
{
    if(!expr || expr->status()!=Cantor::Expression::Done || expr->results().isEmpty()) {
        return QList<Cantor::DefaultVariableModel::Variable>();
    }

    //for parsing of names and values below (old code) we need to combine multiple results back to one string
    QString text;
    for (auto* result : expr->results())
    {
        if(result->type()==Cantor::TextResult::Type)
            text += dynamic_cast<Cantor::TextResult*>(result)->plain();
        else if(expr->result()->type()==Cantor::LatexResult::Type)
            text += dynamic_cast<Cantor::LatexResult*>(result)->plain();
    }

    const int nameIndex=text.indexOf(QLatin1Char(']'));
    QString namesString=text.left(nameIndex);
    //namesString.chop(1);
    namesString=namesString.mid(1);
    namesString=namesString.trimmed();

    qDebug()<<"variable names: "<<namesString;
    if(namesString.isEmpty())
        return QList<Cantor::DefaultVariableModel::Variable>();

    QStringList variableNames;
    QString valuesString;
    bool hasValues = false;
    QStringList variableValues;
    if ( namesString.contains(QLatin1Char(')')) )
    {
        //function definition(s): e.g
        //text = "[f1(x),f2(x,y),f3(x,y,z)]\n$DONE"
        //nameString = f1(x),f2(x,y),f3(x,y,z)
        //variableString = "\n$DONE"
        variableNames = namesString.split(QLatin1String("),"));
    }
    else
    {
        //variable definition(s): e.g.
        //text = "[a,b]\n1\n\"-cantor-value-separator-\"\n2\n\"-cantor-value-separator-\"\n($A $B)"
        //nameString = "[a,b]"
        //variableString = "\n1\n\"-cantor-value-separator-\"\n2\n\"-cantor-value-separator-\"\n($A $B)"
        variableNames = namesString.split(QLatin1Char(','));
        valuesString = text.mid(nameIndex+1).trimmed();
        valuesString = valuesString.remove(QLatin1String("\n")); //lists with many elements have line breaks, remove them
        variableValues = valuesString.split(QLatin1String("\"-cantor-value-separator-\""));
        hasValues = variableValues.isEmpty();
    }

    qDebug()<<variableNames;
    qDebug()<<"string: "<<valuesString;
    qDebug()<<"values: "<<variableValues;
    qDebug()<<"has Values: "<<hasValues;

    QList<Cantor::DefaultVariableModel::Variable> variables;
    variables.reserve(variableNames.size());
    for(int i=0;i<variableNames.size();i++)
    {
        Cantor::DefaultVariableModel::Variable var;
        var.name=variableNames.at(i);;
        if(variableValues.size()>i)
        {
            var.value=variableValues.at(i).trimmed();
            var.value=var.value.remove(QLatin1String("\n")); //lists with many elements have line breaks, remove them
        }
        else
            var.value=QLatin1String("unknown");

        variables<<var;
    }

    return variables;
}

void MaximaVariableModel::parseNewVariables(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Done && status != Cantor::Expression::Error)
        return;

    qDebug()<<"parsing variables";
    MaximaExpression* expr=dynamic_cast<MaximaExpression*>(sender());

    QList<Variable> newVars=parse(expr);
    QStringList addedVars;
    QStringList removedVars;
    //remove the old variables
    for (const Variable& var : m_variables)
    {
        //check if this var is present in the new variables
        bool found=false;
        for (const Variable& var2 : newVars)
        {
            if(var.name==var2.name)
            {
                found=true;
                break;
            }
        }

        if(!found)
        {
            removeVariable(var);
            removedVars<<var.name;
        }
    }

    for (const Variable& var : newVars)
    {
        addVariable(var);

        addedVars<<var.name;
    }

    m_variables=newVars;

    //the expression is not needed anymore
    expr->deleteLater();

    emit variablesAdded(addedVars);
    emit variablesRemoved(removedVars);
}

void MaximaVariableModel::parseNewFunctions(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Done && status != Cantor::Expression::Error)
        return;

    qDebug()<<"parsing functions";
    MaximaExpression* expr=dynamic_cast<MaximaExpression*>(sender());

    QList<Variable> newVars=parse(expr);
    QStringList addedVars;
    QStringList removedVars;

    //remove the old variables
    for (const Variable& var : m_functions)
    {
        //check if this var is present in the new variables
        bool found=false;
        for (const Variable& var2 : newVars)
        {
            if(var.name==var2.name)
            {
                found=true;
                break;
            }
        }

        if(!found)
        {
            removeVariable(var);
            removedVars<<var.name;
        }
    }

    for (Variable var : newVars)
    {
        var.value=i18n("function");
        addVariable(var);
        //todo: check if the variable is actually new?
        addedVars<<var.name;
    }

    m_functions=newVars;

    //the expression is not needed anymore
    expr->deleteLater();

    emit functionsAdded(addedVars);
    emit functionsRemoved(removedVars);
}

MaximaSession* MaximaVariableModel::maximaSession()
{
    return static_cast<MaximaSession*> (session());
}

QList<Cantor::DefaultVariableModel::Variable> MaximaVariableModel::variables()
{
    return m_variables;
}

QList<Cantor::DefaultVariableModel::Variable> MaximaVariableModel::functions()
{
    return m_functions;
}

QStringList MaximaVariableModel::variableNames()
{
    QStringList names;
    for (const Cantor::DefaultVariableModel::Variable& var : m_variables)
        names<<var.name;

    return names;
}

QStringList MaximaVariableModel::functionNames(bool stripParameters)
{
    QStringList names;
    for (const Cantor::DefaultVariableModel::Variable& var : m_functions)
    {
        QString name=var.name;
        if(stripParameters)
        {
            name=name.left(name.lastIndexOf(QLatin1Char('(')));
        }
        names<<name;
    }

    return names;
}
