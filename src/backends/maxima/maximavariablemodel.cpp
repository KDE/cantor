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


//command used to inspect a maxima variable. %1 is the name of that variable
const QString MaximaVariableModel::inspectCommand=":lisp($disp $%1)";

MaximaVariableModel::MaximaVariableModel( MaximaSession* session) : Cantor::DefaultVariableModel(session)
{

}

MaximaVariableModel::~MaximaVariableModel()
{

}



void MaximaVariableModel::checkForNewVariables()
{
    kDebug()<<"checking for new variables";
    const QString& cmd=inspectCommand.arg("values");
    Cantor::Expression* expr=session()->evaluateExpression(cmd);

    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(parseNewVariables()));
}

void MaximaVariableModel::checkForNewFunctions()
{
    kDebug()<<"checking for new functions";
    const QString& cmd=inspectCommand.arg("functions");
    Cantor::Expression* expr=session()->evaluateExpression(cmd);

    connect(expr, SIGNAL(statusChanged(Cantor::Expression::Status)), this, SLOT(parseNewFunctions()));
}

QList<Cantor::DefaultVariableModel::Variable> parse(MaximaExpression* expr)
{
    kDebug()<<"parsing it!";
    if(!expr||expr->status()!=Cantor::Expression::Done)
        return QList<Cantor::DefaultVariableModel::Variable>();

    QString text;
    if(expr->result()->type()==Cantor::TextResult::Type)
        text=dynamic_cast<Cantor::TextResult*>(expr->result())->plain();
    else if(expr->result()->type()==Cantor::LatexResult::Type)
        text=dynamic_cast<Cantor::LatexResult*>(expr->result())->plain();
    else
    {
        kDebug()<<"unsupportet type: "<<expr->result()->type()<<endl;
        return QList<Cantor::DefaultVariableModel::Variable>();
    }


    kDebug()<<"got "<<text;

    text.chop(1);
    text=text.mid(1);
    text=text.trimmed();

    if(text.isEmpty())
        return QList<Cantor::DefaultVariableModel::Variable>();

    QStringList variableNames=text.trimmed().split(',');

    kDebug()<<variableNames;

    QList<Cantor::DefaultVariableModel::Variable> variables;
    variables.reserve(variableNames.size());
    foreach(const QString& name, variableNames)
    {
        Cantor::DefaultVariableModel::Variable var;
        var.name=name;
        var.value="unknown";

        variables<<var;

        //TODO: get the values of the variables
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

    foreach(const Variable& var, newVars)
    {
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
