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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "sageextensions.h"
#include <QStringList>
#include <KLocale>

#define SAGE_EXTENSION_CONSTRUCTORS(name) Sage##name##Extension::Sage##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Sage##name##Extension::~Sage##name##Extension() {}


//History Extension
SAGE_EXTENSION_CONSTRUCTORS(History)

QString SageHistoryExtension::lastResult()
{
    return QLatin1String("_");
}

//Script Extension
SAGE_EXTENSION_CONSTRUCTORS(Script)

QString SageScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("execfile(\"%1\")").arg(path);
}

QString SageScriptExtension::scriptFileFilter()
{
    return i18n("Python script file (*.py);;Sage script file( *.sage)");
}

QString SageScriptExtension::highlightingMode()
{
    return QLatin1String("python");
}

//CAS extension
SAGE_EXTENSION_CONSTRUCTORS(CAS)

QString SageCASExtension::solve(const QStringList& equations, const QStringList& variables)
{
    QString eqstr=QString::fromLatin1("[%1]").arg(equations.join(QLatin1String(",")));
    eqstr.replace(QLatin1Char('='), QLatin1String("==")); //Sage uses == for equations and = for assignments
    QString variablestr=variables.join(QLatin1String(","));

    return QString::fromLatin1("solve(%1,%2)").arg(eqstr, variablestr);
}

QString SageCASExtension::simplify(const QString& expression)
{
    return QString::fromLatin1("simplify(%1)").arg(expression);
}

QString SageCASExtension::expand(const QString& expression)
{
    return QString::fromLatin1("expand(%1)").arg(expression);
}

//Calculus Extension
SAGE_EXTENSION_CONSTRUCTORS(Calculus)

QString SageCalculusExtension::limit(const QString& expression, const QString& variable, const QString& limit)
{
    return QString::fromLatin1("limit(%1,%2=%3)").arg(expression, variable, limit);
}

QString SageCalculusExtension::differentiate(const QString& function,const QString& variable, int times)
{
    return QString::fromLatin1("diff(%1,%2,%3)").arg(function, variable, QString::number(times));
}

QString SageCalculusExtension::integrate(const QString& function, const QString& variable)
{
    return QString::fromLatin1("integral(%1,%2)").arg(function, variable);
}

QString SageCalculusExtension::integrate(const QString& function,const QString& variable, const QString& left, const QString& right)
{
    return QString::fromLatin1("integral(%1,%2,%3,%4)").arg(function, variable, left, right);
}

//Linear Algebra
SAGE_EXTENSION_CONSTRUCTORS(LinearAlgebra)

QString SageLinearAlgebraExtension::createVector(const QStringList& entries, VectorType type)
{
    QString cmd=QLatin1String("vector(");
    foreach(const QString& e, entries)
        cmd+=e+QLatin1Char(',');
    cmd.chop(1);
    cmd+=QLatin1Char(')');

    if(type==Cantor::LinearAlgebraExtension::ColumnVector)
        cmd+=QLatin1String(".transpose()");

    return cmd;
}

QString SageLinearAlgebraExtension::nullVector(int size, VectorType type)
{
    QString cmd=QString::fromLatin1("vector(seq(0 for i in range(0,%1)))").arg(size);
    if(type==Cantor::LinearAlgebraExtension::ColumnVector)
        cmd+=QLatin1String(".transpose()");

    return cmd;
}

QString SageLinearAlgebraExtension::createMatrix(const Matrix& matrix)
{
    QString cmd=QLatin1String("matrix([");
    foreach(const QStringList& row, matrix)
    {
        cmd+=QLatin1Char('[');
        foreach(const QString& entry, row)
            cmd+=entry+QLatin1Char(',');
        cmd.chop(1);
        cmd+=QLatin1String("],");
    }
    cmd.chop(1);
    cmd+=QLatin1String("])");

    return cmd;
}

QString SageLinearAlgebraExtension::identityMatrix(int size)
{
    return QString::fromLatin1("identity_matrix(%1)").arg(size);
}

QString SageLinearAlgebraExtension::nullMatrix(int rows,int columns)
{
    return QString::fromLatin1("null_matrix(%1,%2)").arg(rows, columns);
}

QString SageLinearAlgebraExtension::rank(const QString& matrix)
{
    return QString::fromLatin1("%1.rank()").arg(matrix);
}

QString SageLinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString::fromLatin1("%1.inverse()").arg(matrix);
}

QString SageLinearAlgebraExtension::charPoly(const QString& matrix)
{
    return QString::fromLatin1("%1.char_poly()").arg(matrix);
}

QString SageLinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    return QString::fromLatin1("%1.eigenvectors_right()").arg(matrix);
}

QString SageLinearAlgebraExtension::eigenValues(const QString& matrix)
{
    return QString::fromLatin1("%1.eigenvalues()").arg(matrix);
}

//Plotting
SAGE_EXTENSION_CONSTRUCTORS(Plot)

QString SagePlotExtension::plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right)
{
    return QString::fromLatin1("plot(%1,%2,%3,%4)").arg(function, variable, left, right);
}

QString SagePlotExtension::plotFunction3d(const QString& function, VariableParameter var1, VariableParameter var2)
{
    const Interval& int1=var1.second;
    const Interval& int2=var2.second;
    return QString::fromLatin1("plot3d(%1,(%2,%3,%4),(%6,%7,%8))").arg(function,
                                                           var1.first, int1.first, int1.second,
                                                           var2.first, int2.first, int2.second);
}

//Packaging
SAGE_EXTENSION_CONSTRUCTORS(Packaging)

QString SagePackagingExtension::importPackage(const QString& module)
{
    return QString::fromLatin1("import %1").arg(module);
}
