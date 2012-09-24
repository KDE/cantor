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

#include <kdebug.h>
#include <klocale.h>

//command used to inspect a maxima variable. %1 is the name of that variable
const QString MaximaVariableModel::inspectCommand=":lisp($disp $%1)";
const QString MaximaVariableModel::variableInspectCommand=":lisp(cantor-inspect $%1)";

MaximaVariableModel::MaximaVariableModel( MaximaSession* session) : Cantor::DefaultVariableModel(session)
{

}

MaximaVariableModel::~MaximaVariableModel()
{

}



void MaximaVariableModel::checkForNewVariables()
{
    kDebug()<<"checking for new variables";
    const QString& cmd=variableInspectCommand.arg("values");
    Cantor::Expression* expr=session()->evaluateExpression(cmd);
    expr->setInternal(true);
    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(parseNewVariables()));
}

void MaximaVariableModel::checkForNewFunctions()
{
    kDebug()<<"checking for new functions";
    const QString& cmd=inspectCommand.arg("functions");
    Cantor::Expression* expr=session()->evaluateExpression(cmd);
    expr->setInternal(true);

    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(parseNewFunctions()));
}

QList<Cantor::DefaultVariableModel::Variable> parse(MaximaExpression* expr)
{
    // Should we really just use results().at(0) here?
    // It wouldn't be much of a problem to iterate the lists
    kDebug()<<"parsing it!";
    if(!expr||expr->status()!=Cantor::Expression::Done)
        return QList<Cantor::DefaultVariableModel::Variable>();

    QString text;
    if(expr->results().at(0)->type()==Cantor::TextResult::Type)
        text=dynamic_cast<Cantor::TextResult*>(expr->results().at(0))->plain();
    else if(expr->results().at(0)->type()==Cantor::LatexResult::Type)
        text=dynamic_cast<Cantor::LatexResult*>(expr->results().at(0))->plain();
    else
    {
        kDebug()<<"unsupported type: "<<expr->results().at(0)->type()<<endl;
        return QList<Cantor::DefaultVariableModel::Variable>();
    }


    kDebug()<<"got "<<text;

    const int nameIndex=text.indexOf(']');
    kDebug()<<"idx: "<<nameIndex;
    QString namesString=text.left(nameIndex);
    //namesString.chop(1);
    namesString=namesString.mid(1);
    namesString=namesString.trimmed();

    kDebug()<<"names: "<<namesString;
    if(namesString.isEmpty())
        return QList<Cantor::DefaultVariableModel::Variable>();

    QStringList variableNames=namesString.split(',');

    QString valuesString=text.mid(nameIndex+1).trimmed();

    QStringList variableValues=valuesString.split("\"-cantor-value-separator-\"");
    bool hasValues=variableValues.isEmpty();

    kDebug()<<variableNames;
    kDebug()<<"string: "<<valuesString;
    kDebug()<<"values: "<<variableValues;
    kDebug()<<"has Values: "<<hasValues;

    QList<Cantor::DefaultVariableModel::Variable> variables;
    variables.reserve(variableNames.size());
    for(int i=0;i<variableNames.size();i++)
    {
        Cantor::DefaultVariableModel::Variable var;
        var.name=variableNames.at(i);;
        if(variableValues.size()>i)
            var.value=variableValues.at(i).trimmed();
        else
            var.value="unknown";
        variables<<var;

    }

    return variables;
}

void MaximaVariableModel::parseNewVariables()
{
    kDebug()<<"parsing variables";
    MaximaExpression* expr=dynamic_cast<MaximaExpression*>(sender());

    QList<Variable> newVars=parse(expr);

    QStringList addedVars;
    QStringList removedVars;
    //remove the old variables
    foreach(const Variable& var,m_variables)
    {
        //check if this var is present in the new variables
        bool found=false;
        foreach(const Variable& var2, newVars)
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

    foreach(const Variable& var, newVars)
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

void MaximaVariableModel::parseNewFunctions()
{
    kDebug()<<"parsing functions";
    MaximaExpression* expr=dynamic_cast<MaximaExpression*>(sender());

    QList<Variable> newVars=parse(expr);
    QStringList addedVars;
    QStringList removedVars;

    //remove the old variables
    foreach(const Variable& var,m_functions)
    {
        //check if this var is present in the new variables
        bool found=false;
        foreach(const Variable& var2, newVars)
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

    foreach(Variable var, newVars)
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
    foreach(const Cantor::DefaultVariableModel::Variable& var, m_variables)
        names<<var.name;

    return names;
}

QStringList MaximaVariableModel::functionNames(bool stripParameters)
{
    QStringList names;
    foreach(const Cantor::DefaultVariableModel::Variable& var, m_functions)
    {
        QString name=var.name;
        if(stripParameters)
        {
            name=name.left(name.lastIndexOf('('));
        }
        names<<name;
    }

    return names;
}
