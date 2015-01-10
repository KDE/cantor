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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#include "pythonextensions.h"

#include <QDebug>

#include <KLocale>
#include <KStandardDirs>

#include "pythonutils.h"

#define PYTHON_EXT_CDTOR(name) Python##name##Extension::Python##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Python##name##Extension::~Python##name##Extension() {}

PYTHON_EXT_CDTOR(LinearAlgebra)

QString PythonLinearAlgebraExtension::createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command;
    command += QLatin1String("numpy.matrix([");

    foreach (const QString& entry, entries)
    {
        command += entry + QLatin1String(", ");
    }

    command.chop(2);
    command += QLatin1String("])\n");

    return command;
}

QString PythonLinearAlgebraExtension::createMatrix(const Cantor::LinearAlgebraExtension::Matrix& matrix)
{
    QString command;
    command += QLatin1String("numpy.matrix([[");

    foreach (const QStringList row, matrix)
    {
        foreach (const QString entry, row)
        {
            command += entry;
            command += QLatin1String(", ");
        }
        command.chop(2);
        command += QLatin1String("], [");
    }

    command.chop(3);
    command += QLatin1String("])");

    return command;
}

QString PythonLinearAlgebraExtension::eigenValues(const QString& matrix)
{
    return QString::fromLatin1("numpy.linalg.eigvals(%1)").arg(matrix);
}

QString PythonLinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    return QString::fromLatin1("numpy.linalg.eig(%1)").arg(matrix);
}

QString PythonLinearAlgebraExtension::identityMatrix(int size)
{
    return QString::fromLatin1("numpy.identity(%1)").arg(size);
}

QString PythonLinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString::fromLatin1("numpy.linalg.inv(%1)").arg(matrix);
}

QString PythonLinearAlgebraExtension::nullMatrix(int rows, int columns)
{
    return QString::fromLatin1("numpy.zeros(%1, %2)").arg(rows).arg(columns);
}

QString PythonLinearAlgebraExtension::nullVector(int size, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command = QLatin1String("numpy.zeros(%1, %2)");
    switch (type)
    {
        case ColumnVector:
            return command.arg(size).arg(1);
        case RowVector:
            return command.arg(1).arg(size);
        default:
            return Cantor::LinearAlgebraExtension::nullVector(size, type);
    }
}

QString PythonLinearAlgebraExtension::rank(const QString& matrix)
{
    return QString::fromLatin1("numpy.linalg.matrix_rank(%1)").arg(matrix);
}

QString PythonLinearAlgebraExtension::charPoly(const QString& matrix)
{
    return QString::fromLatin1("numpy.poly(%1)").arg(matrix);
}

PYTHON_EXT_CDTOR(Packaging)

QString PythonPackagingExtension::importPackage(const QString& package)
{
    return QString::fromLatin1("import %1").arg(package);
}

PYTHON_EXT_CDTOR(Plot)

QString PythonPlotExtension::plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right)
{
    QString argumentToPlot = variable;
    QString xlimits;

    if(!function.isEmpty()){
        argumentToPlot = function + QLatin1String("(") + variable + QLatin1String(")");
    }

    if(!left.isEmpty() && !right.isEmpty()){
        xlimits = QString::fromLatin1("pylab.xlim(%1, %2)\n").arg(left).arg(right);
    }

    return QString::fromLatin1("pylab.clf()\n"                     \
                               "pylab.plot(%1)\n"                  \
                               "%2"                                \
                               "pylab.show()").arg(argumentToPlot).arg(xlimits);
}

QString PythonPlotExtension::plotFunction3d(const QString& function, Cantor::PlotExtension::VariableParameter var1, Cantor::PlotExtension::VariableParameter var2)
{
    const Interval& interval1 = var1.second;
    const Interval& interval2 = var2.second;

    QString interval1Limits;
    QString interval2Limits;

    if(!interval1.first.isEmpty() && !interval1.second.isEmpty()){
        interval1Limits = QString::fromLatin1("ax3D.set_xlim3d(%1, %2)\n").arg(interval1.first).arg(interval1.second);
    }

    if(!interval2.first.isEmpty() && !interval2.second.isEmpty()){
        interval2Limits = QString::fromLatin1("ax3D.set_ylim3d(%1, %2)\n").arg(interval2.first).arg(interval2.second);
    }

    return QString::fromLatin1("from mpl_toolkits.mplot3d import Axes3D\n\n"                      \
                               "fig3D = pylab.figure()\n"                                         \
                               "ax3D = fig3D.gca(projection='3d')\n"                              \
                               "ax3D.plot_surface(%1, %2, %3(%1, %2), rstride=4, cstride=4)\n"    \
                               "%4%5"                                                             \
                               "pylab.show()").arg(var1.first).arg(var2.first).arg(function)
                                   .arg(interval1Limits).arg(interval2Limits);
}

PYTHON_EXT_CDTOR(Script)

QString PythonScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("execfile(\"%1\")").arg(path);
}

QString PythonScriptExtension::scriptFileFilter()
{
    return i18n("*.py|Python script file");
}

QString PythonScriptExtension::highlightingMode()
{
    return QLatin1String("python");
}

PYTHON_EXT_CDTOR(VariableManagement)

QString PythonVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name, value);
}

QString PythonVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString::fromLatin1("%1 = %2").arg(name).arg(value);
}

QString PythonVariableManagementExtension::removeVariable(const QString& name)
{
    return QString::fromLatin1("del(%1)").arg(name);
}

QString PythonVariableManagementExtension::clearVariables()
{
    const QString& variablesCleanerFile = KStandardDirs::locate("appdata",
                                                                QLatin1String("pythonbackend/variables_cleaner.py"));
    return fromSource(variablesCleanerFile);
}

QString PythonVariableManagementExtension::saveVariables(const QString& fileName)
{
    const QString& variablesSaverFile = KStandardDirs::locate("appdata",
                                                              QLatin1String("pythonbackend/variables_saver.py"));

    return fromSource(variablesSaverFile).arg(fileName);
}

QString PythonVariableManagementExtension::loadVariables(const QString& fileName)
{
    const QString& variablesLoaderFile = KStandardDirs::locate("appdata",
                                                               QLatin1String("pythonbackend/variables_loader.py"));

    return fromSource(variablesLoaderFile).arg(fileName);
}
