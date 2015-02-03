/************************************************************************************
*  Copyright (C) 2011 by Matteo Agostinelli <agostinelli@gmail.com>                 *
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

#include "qalculateextensions.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Variable.h>

#include <QStringList>
#include <KLocale>

#define QALCULATE_EXT_CDTOR(name) Qalculate##name##Extension::Qalculate##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                  Qalculate##name##Extension::~Qalculate##name##Extension() {}

QALCULATE_EXT_CDTOR(History)
QALCULATE_EXT_CDTOR(VariableManagement)
QALCULATE_EXT_CDTOR(CAS)
QALCULATE_EXT_CDTOR(Calculus)
QALCULATE_EXT_CDTOR(LinearAlgebra)

QString QalculateHistoryExtension::lastResult()
{
    return QLatin1String("ans");
}

QString QalculateVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name,value);
}

QString QalculateVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString::fromLatin1("%1 := %2").arg(name).arg(value);
}

QString QalculateVariableManagementExtension::removeVariable(const QString& name)
{
    Q_UNUSED(name)
    //CALCULATOR->getVariable(name.toStdString())->setActive(false);
    return QString();
}

QString QalculateVariableManagementExtension::clearVariables()
{
    //CALCULATOR->resetVariables();
    return QString();
}

QString QalculateVariableManagementExtension::saveVariables(const QString& fileName)
{
    QString escapedFileName = fileName;
    escapedFileName.replace(QLatin1Char(' '), QLatin1String("\\ "));
    return QString::fromLatin1("saveVariables %1").arg(escapedFileName);
}

QString QalculateVariableManagementExtension::loadVariables(const QString& fileName)
{
    QString escapedFileName = fileName;
    escapedFileName.replace(QLatin1Char(' '), QLatin1String("\\ "));
    return QString::fromLatin1("loadVariables %1").arg(escapedFileName);
}

// Custom Plot Extension. This extension does not fit into the normal pattern,
// because it inherts Cantor::Extension directly.
// Also it does not do anything at all, because all the work is done by the
// QalculatePlotAssistant
QalculatePlotExtension::QalculatePlotExtension(QObject* parent) :
    Cantor::Extension(QLatin1String("QalculatePlotExtension"), parent)
{
}

QalculatePlotExtension::~QalculatePlotExtension()
{
}

QString QalculateCASExtension::solve(const QStringList& equations, const QStringList& variables)
{
    QString eqstr=QString::fromLatin1("[%1]").arg(equations.join(QLatin1String(",")));

    QString variablestr=QString::fromLatin1("[%1]").arg(variables.join(QLatin1String(",")));

    return QString::fromLatin1("multisolve(%1,%2)").arg(eqstr, variablestr);
}

QString QalculateCASExtension::simplify(const QString& expression)
{
    // There is (currently) no way to do this
    return QString::fromLatin1("").arg(expression);
}

QString QalculateCASExtension::expand(const QString& expression)
{
    // There is (currently) no way to do this
    return QString::fromLatin1("").arg(expression);
}

QString QalculateCalculusExtension::limit(const QString& expression, const QString& variable, const QString& limit)
{
    Q_UNUSED(expression)
    Q_UNUSED(variable)
    Q_UNUSED(limit)
    // There is no limit function in Qalculate (at least none I know of),
    // but fortunately this function seems not to be used anyway.
    return QLatin1String("");
    //return QString("limit(%1, %2=%3);").arg(expression, variable, limit);
}

QString QalculateCalculusExtension::differentiate(const QString& function,const QString& variable, int times)
{
    return QString::fromLatin1("diff(%1, %2, %3)").arg(function, variable, QString::number(times));
}

QString QalculateCalculusExtension::integrate(const QString& function, const QString& variable)
{
    return QString::fromLatin1("integrate(%1, %2)").arg(function, variable);
}

QString QalculateCalculusExtension::integrate(const QString& function,const QString& variable, const QString& left, const QString& right)
{
    return QString::fromLatin1("integrate(%1, %2, %3, %4)").arg(function, variable, left, right);
}

//Commands to create Vectors/Matrices
QString QalculateLinearAlgebraExtension::createVector(const QStringList& entries, VectorType type)
{
    // Neither of these does create a normal vector, but a n-times-1 or
    // an 1-times-n matrix.
    if(type==Cantor::LinearAlgebraExtension::ColumnVector) {
        QString list=entries.join(QLatin1String("], ["));
        return QString::fromLatin1("[[%1]]").arg(list);
    }
    else {
        QString list=entries.join(QLatin1String(","));
        return QString::fromLatin1("[[%1]]").arg(list);
    }
}

QString QalculateLinearAlgebraExtension::createMatrix(const Matrix& matrix)
{
  QString cmd=QLatin1String("[");
    foreach(const QStringList& row, matrix)
    {
        cmd+=QLatin1Char('[');
        foreach(const QString& entry, row)
            cmd+=entry+QLatin1Char(',');
        cmd.chop(1);
        cmd+=QLatin1String("],");
    }
    cmd.chop(1);
    cmd+=QLatin1String("]");

    return cmd;
}

QString QalculateLinearAlgebraExtension::identityMatrix(int size)
{
    return QString::fromLatin1("identity(%1)").arg(size);
}

//basic functions
QString QalculateLinearAlgebraExtension::rank(const QString& matrix)
{
    // This feature seems to be missing in Qalculate
    return QString::fromLatin1("").arg(matrix);
}

QString QalculateLinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString::fromLatin1("inverse(%1)").arg(matrix);
}

QString QalculateLinearAlgebraExtension::charPoly(const QString& matrix)
{
  return QString::fromLatin1("det(x*identity(%1)-%2)").arg(matrix, matrix);
}

QString QalculateLinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    // No such function
    return QString::fromLatin1("").arg(matrix);
}

QString QalculateLinearAlgebraExtension::eigenValues(const QString& matrix)
{
    // No such function
    return QString::fromLatin1("").arg(matrix);
}
