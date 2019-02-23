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

#include "settings.h"

//command used to inspect a maxima variable. %1 is the name of that variable
const QString MaximaVariableModel::inspectCommand=QLatin1String(":lisp($disp $%1)");
const QString MaximaVariableModel::variableInspectCommand=QLatin1String(":lisp(cantor-inspect $%1)");

MaximaVariableModel::MaximaVariableModel( MaximaSession* session) : Cantor::DefaultVariableModel(session),
m_variableExpression(nullptr),
m_functionExpression(nullptr)
{
}

void MaximaVariableModel::update()
{
    if (!m_variableExpression)
    {
        qDebug()<<"checking for new variables";
        const QString& cmd1=variableInspectCommand.arg(QLatin1String("values"));
        m_variableExpression = session()->evaluateExpression(cmd1, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_variableExpression, &Cantor::Expression::statusChanged, this, &MaximaVariableModel::parseNewVariables);
    }

    if (!m_functionExpression)
    {
        qDebug()<<"checking for new functions";
        const QString& cmd2=inspectCommand.arg(QLatin1String("functions"));
        m_functionExpression = session()->evaluateExpression(cmd2, Cantor::Expression::FinishingBehavior::DoNotDelete, true);
        connect(m_functionExpression, &Cantor::Expression::statusChanged, this, &MaximaVariableModel::parseNewFunctions);
    }
}

QList<Cantor::DefaultVariableModel::Variable> parse(MaximaExpression* expr)
{
    if(!expr
        || (expr->status()!=Cantor::Expression::Done && expr->errorMessage() != QLatin1String("$DONE"))
        || expr->results().isEmpty())
    {
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
        if (MaximaSettings::self()->variableManagement())
        {
            valuesString = text.mid(nameIndex+1).trimmed();
            valuesString = valuesString.remove(QLatin1String("\n")); //lists with many elements have line breaks, remove them
            variableValues = valuesString.split(QLatin1String("\"-cantor-value-separator-\""));
            hasValues = variableValues.isEmpty();
        }
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

    QList<Variable> newVars=parse(static_cast<MaximaExpression*>(m_variableExpression));
    setVariables(newVars);

    //the expression is not needed anymore
    m_variableExpression->deleteLater();
    m_variableExpression = nullptr;
}

void MaximaVariableModel::parseNewFunctions(Cantor::Expression::Status status)
{
    if (status != Cantor::Expression::Done && status != Cantor::Expression::Error)
        return;

    qDebug()<<"parsing functions";

    // List of variables?
    QList<Variable> newFuncs=parse(static_cast<MaximaExpression*>(m_functionExpression));
    QStringList functions;
    for (Variable var : newFuncs)
        functions << var.name;
    setFunctions(functions);

    //the expression is not needed anymore
    m_functionExpression->deleteLater();
    m_functionExpression = nullptr;
}

MaximaSession* MaximaVariableModel::maximaSession()
{
    return static_cast<MaximaSession*> (session());
}

QStringList MaximaVariableModel::functionNames(bool stripParameters)
{
    QStringList names;
    for (const QString func: m_functions)
    {
        QString name=func;
        if(stripParameters)
        {
            name=name.left(name.lastIndexOf(QLatin1Char('(')));
        }
        names<<name;
    }

    return names;
}
