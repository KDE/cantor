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

#include "maximaextensions.h"
#include <QStringList>
#include <KLocale>

#define MAXIMA_EXTENSION_CONSTRUCTORS(name) Maxima##name##Extension::Maxima##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Maxima##name##Extension::~Maxima##name##Extension() {}

//History Extension
MAXIMA_EXTENSION_CONSTRUCTORS(History)

QString MaximaHistoryExtension::lastResult()
{
    return QLatin1String("%");
}

//Script
MAXIMA_EXTENSION_CONSTRUCTORS(Script)

QString MaximaScriptExtension::runExternalScript(const QString& file)
{
    return QString::fromLatin1("batch(\"%1\")$").arg(file);
}

QString MaximaScriptExtension::scriptFileFilter()
{
    return i18n("Maxima batch file (*.mac)");
}

QString MaximaScriptExtension::highlightingMode()
{
    return QLatin1String("maxima");
}

QString MaximaScriptExtension::commentStartingSequence()
{
    return QLatin1String("/* ");
}

QString MaximaScriptExtension::commentEndingSequence()
{
    return QLatin1String(" */");
}

//CAS Extension
MAXIMA_EXTENSION_CONSTRUCTORS(CAS)

QString MaximaCASExtension::solve(const QStringList& equations, const QStringList& variables)
{
    QString eqstr=QString::fromLatin1("[%1]").arg(equations.join(QLatin1String(",")));

    QString variablestr=QString::fromLatin1("[%1]").arg(variables.join(QLatin1String(",")));

    return QString::fromLatin1("solve(%1,%2);").arg(eqstr, variablestr);
}

QString MaximaCASExtension::simplify(const QString& expression)
{
    return QString::fromLatin1("simplify(%1);").arg(expression);
}

QString MaximaCASExtension::expand(const QString& expression)
{
    return QString::fromLatin1("expand(%1);").arg(expression);
}

//Calculus Extension
MAXIMA_EXTENSION_CONSTRUCTORS(Calculus)

QString MaximaCalculusExtension::limit(const QString& expression, const QString& variable, const QString& limit)
{
    return QString::fromLatin1("limit(%1, %2=%3);").arg(expression, variable, limit);
}

QString MaximaCalculusExtension::differentiate(const QString& function,const QString& variable, int times)
{
    return QString::fromLatin1("diff(%1, %2, %3);").arg(function, variable, QString::number(times));
}

QString MaximaCalculusExtension::integrate(const QString& function, const QString& variable)
{
    return QString::fromLatin1("integrate(%1, %2);").arg(function, variable);
}

QString MaximaCalculusExtension::integrate(const QString& function,const QString& variable, const QString& left, const QString& right)
{
    return QString::fromLatin1("integrate(%1, %2, %3, %4);").arg(function, variable, left, right);
}

//Linear Algebra Extension
MAXIMA_EXTENSION_CONSTRUCTORS(LinearAlgebra)

//Commands to create Vectors/Matrices
QString MaximaLinearAlgebraExtension::createVector(const QStringList& entries, VectorType type)
{
    QString list=entries.join(QLatin1String(","));

    if(type==Cantor::LinearAlgebraExtension::ColumnVector)
        return QString::fromLatin1("columnvector([%1]);").arg(list);
    else
        return QString::fromLatin1("rowvector([%1]);").arg(list);
}

QString MaximaLinearAlgebraExtension::createMatrix(const Matrix& matrix)
{
    QString cmd=QLatin1String("matrix(");
    foreach(const QStringList& row, matrix)
    {
        cmd+=QLatin1Char('[');
        foreach(const QString& entry, row)
            cmd+=entry+QLatin1Char(',');
        cmd.chop(1);
        cmd+=QLatin1String("],");
    }
    cmd.chop(1);
    cmd+=QLatin1String(");");

    return cmd;
}

QString MaximaLinearAlgebraExtension::identityMatrix(int size)
{
    return QString::fromLatin1("ident(%1);").arg(size);
}

//basic functions
QString MaximaLinearAlgebraExtension::rank(const QString& matrix)
{
    return QString::fromLatin1("rank(%1);").arg(matrix);
}

QString MaximaLinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString::fromLatin1("invert(%1);").arg(matrix);
}

QString MaximaLinearAlgebraExtension::charPoly(const QString& matrix)
{
    return QString::fromLatin1("charpoly(%1,x);").arg(matrix);
}

QString MaximaLinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    return QString::fromLatin1("eigenvectors(%1);").arg(matrix);
}

QString MaximaLinearAlgebraExtension::eigenValues(const QString& matrix)
{
    return QString::fromLatin1("eigenvalues(%1);").arg(matrix);
}

//Plotting
MAXIMA_EXTENSION_CONSTRUCTORS(Plot)

QString MaximaPlotExtension::plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right)
{
    return QString::fromLatin1("plot2d(%1,[%2,%3,%4])").arg(function, variable, left, right);
}

QString MaximaPlotExtension::plotFunction3d(const QString& function, VariableParameter var1, VariableParameter var2)
{
    const Interval& int1=var1.second;
    const Interval& int2=var2.second;
    return QString::fromLatin1("plot3d(%1,[%2,%3,%4],[%6,%7,%8])").arg(function,
                                                           var1.first, int1.first, int1.second,
                                                           var2.first, int2.first, int2.second);
}

//Variable Management
MAXIMA_EXTENSION_CONSTRUCTORS(VariableManagement)

QString MaximaVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return QString::fromLatin1("%1: %2").arg(name).arg(value);
}

QString MaximaVariableManagementExtension::setValue(const QString& name,const QString& value)
{
    return QString::fromLatin1("%1: %2").arg(name).arg(value);
}

QString MaximaVariableManagementExtension::removeVariable(const QString& name)
{
    return QString::fromLatin1("kill(%1)").arg(name);
}

QString MaximaVariableManagementExtension::saveVariables(const QString& fileName)
{
    return QString::fromLatin1("save(\"%1\", values,functions)").arg(fileName);
}

QString MaximaVariableManagementExtension::loadVariables(const QString& fileName)
{
    return QString::fromLatin1("load(\"%1\")").arg(fileName);
}

QString MaximaVariableManagementExtension::clearVariables()
{
    return QLatin1String("kill(all)");
}
