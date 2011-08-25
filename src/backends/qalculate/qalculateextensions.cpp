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

#include "qalculateextensions.h"
#include <QStringList>
#include <klocale.h>

#define QALCULATE_EXTENSION_CONSTRUCTORS(name) Qalculate##name##Extension::Qalculate##name##Extension(QObject* parent) : name##Extension(parent) {} \
  Qalculate##name##Extension::~Qalculate##name##Extension() {}


//CAS Extension
QALCULATE_EXTENSION_CONSTRUCTORS(CAS)

QString QalculateCASExtension::solve(const QStringList& equations, const QStringList& variables)
{
    QString eqstr=QString("[%1]").arg(equations.join(","));

    QString variablestr=QString("[%1]").arg(variables.join(","));

    return QString("multisolve(%1,%2)").arg(eqstr, variablestr);
}

QString QalculateCASExtension::simplify(const QString& expression)
{
    // There is (currently) no way to do this
    return QString("").arg(expression);
}

QString QalculateCASExtension::expand(const QString& expression)
{
    // There is (currently) no way to do this
    return QString("").arg(expression);
}

//Calculus Extension
QALCULATE_EXTENSION_CONSTRUCTORS(Calculus)

QString QalculateCalculusExtension::limit(const QString& expression, const QString& variable, const QString& limit)
{
    // There is no limit function in Qalculate (at least none I know of),
    // but fortunately this function seems not to be used anyway.
    return "";
    //return QString("limit(%1, %2=%3);").arg(expression, variable, limit);
}

QString QalculateCalculusExtension::differentiate(const QString& function,const QString& variable, int times)
{
    return QString("diff(%1, %2, %3)").arg(function, variable, QString::number(times));
}

QString QalculateCalculusExtension::integrate(const QString& function, const QString& variable)
{
    return QString("integrate(%1, %2)").arg(function, variable);
}

QString QalculateCalculusExtension::integrate(const QString& function,const QString& variable, const QString& left, const QString& right)
{
    return QString("integrate(%1, %2, %3, %4)").arg(function, variable, left, right);
}


//Linear Algebra Extension
QALCULATE_EXTENSION_CONSTRUCTORS(LinearAlgebra)

//Commands to create Vectors/Matrices
QString QalculateLinearAlgebraExtension::createVector(const QStringList& entries, VectorType type)
{
    // Neither of these does create a normal vector, but a n-times-1 or
    // an 1-times-n matrix.
    if(type==Cantor::LinearAlgebraExtension::ColumnVector) {
        QString list=entries.join("], [");
        return QString("[[%1]]").arg(list);
    }
    else {
        QString list=entries.join(",");
        return QString("[[%1]]").arg(list);
    }
}

QString QalculateLinearAlgebraExtension::createMatrix(const Matrix& matrix)
{
  QString cmd="[";
    foreach(const QStringList& row, matrix)
    {
        cmd+='[';
        foreach(const QString& entry, row)
            cmd+=entry+',';
        cmd.chop(1);
        cmd+="],";
    }
    cmd.chop(1);
    cmd+="]";

    return cmd;
}

QString QalculateLinearAlgebraExtension::identityMatrix(int size)
{
    return QString("identity(%1)").arg(size);
}

//basic functions
QString QalculateLinearAlgebraExtension::rank(const QString& matrix)
{
    // This feature seems to be missing in Qalculate
    return QString("").arg(matrix);
}

QString QalculateLinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString("inverse(%1)").arg(matrix);
}

QString QalculateLinearAlgebraExtension::charPoly(const QString& matrix)
{
  return QString("det(x*identity(%1)-%2)").arg(matrix, matrix);
}

QString QalculateLinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    // No such function
    return QString("").arg(matrix);
}

QString QalculateLinearAlgebraExtension::eigenValues(const QString& matrix)
{
    // No such function
    return QString("").arg(matrix);
}
