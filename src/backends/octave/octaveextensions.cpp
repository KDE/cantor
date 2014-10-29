/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#include "octaveextensions.h"
#include <KLocale>
#include <QDebug>

#define OCTAVE_EXT_CDTOR(name) Octave##name##Extension::Octave##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Octave##name##Extension::~Octave##name##Extension() {}


const QList<QChar> octaveMatrixOperators = QList<QChar>() << QLatin1Char('*') << QLatin1Char('/') << QLatin1Char('^');
const QString octavePrintFileCommand = QLatin1String("print('-depsc',strcat(tempname(tempdir,'c-ob-'),'.eps')); ");

OCTAVE_EXT_CDTOR(History)

QString OctaveHistoryExtension::lastResult()
{
    return QLatin1String("ans");
}

OCTAVE_EXT_CDTOR(Script)

QString OctaveScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("source \"%1\"").arg(path);
}

QString OctaveScriptExtension::scriptFileFilter()
{
    return i18n("*.m|Octave script file");
}

QString OctaveScriptExtension::highlightingMode()
{
    return QLatin1String("octave");
}

QString OctaveScriptExtension::commandSeparator()
{
    return QLatin1String(";");
}

OCTAVE_EXT_CDTOR(Plot)

QString OctavePlotExtension::plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right)
{
    return QString::fromLatin1("cantor_plot2d('%1','%2',%3,%4);")
		    .arg(function)
		    .arg(variable)
		    .arg(left)
		    .arg(right);
}

QString OctavePlotExtension::plotFunction3d(const QString& function, Cantor::PlotExtension::VariableParameter var1, Cantor::PlotExtension::VariableParameter var2)
{
  return QString::fromLatin1("cantor_plot3d('%1','%2',%3,%4,'%5',%6,%7);")
		  .arg(function)
		  .arg(var1.first)
		  .arg(var1.second.first)
		  .arg(var1.second.second)
		  .arg(var2.first)
		  .arg(var2.second.first)
		  .arg(var2.second.second);
}


OCTAVE_EXT_CDTOR(LinearAlgebra)

QString OctaveLinearAlgebraExtension::charPoly(const QString& matrix)
{
    return QString::fromLatin1("poly(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::createMatrix(const Cantor::LinearAlgebraExtension::Matrix& matrix)
{
    QString command;
    command += QLatin1Char('[');
    foreach (const QStringList row, matrix)
    {
        foreach (const QString entry, row)
        {
            command += entry;
            command += QLatin1String(", ");
        }
        command.chop(2); // Removes the last comma
        command += QLatin1String("; ");
    }
    command.chop(2); // Removes the last semicolon
    command += QLatin1Char(']');
    return command;
}

QString OctaveLinearAlgebraExtension::createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString separator;
    if (type == ColumnVector)
    {
        separator = QLatin1String("; ");
    }
    else
    {
        separator = QLatin1String(", ");
    }
    QString command;
    command += QLatin1Char('[');
    foreach (const QString& entry, entries)
    {
        command += entry;
        command += separator;
    }
    command.chop(1);
    command += QLatin1Char(']');
    return command;
}

QString OctaveLinearAlgebraExtension::eigenValues(const QString& matrix)
{
    return QString::fromLatin1("eig(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    return QString::fromLatin1("cantor_eigenvectors(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::identityMatrix(int size)
{
    return QString::fromLatin1("eye(%1)").arg(size);
}

QString OctaveLinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString::fromLatin1("inv(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::nullMatrix(int rows, int columns)
{
    return QString::fromLatin1("zeros(%1,%2)").arg(rows).arg(columns);
}

QString OctaveLinearAlgebraExtension::nullVector(int size, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command = QLatin1String("zeros(%1,%2)");
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

QString OctaveLinearAlgebraExtension::rank(const QString& matrix)
{
    return QString::fromLatin1("rank(%1)").arg(matrix);
}

OCTAVE_EXT_CDTOR(VariableManagement)

QString OctaveVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name,value);
}

QString OctaveVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString::fromLatin1("%1 = %2").arg(name).arg(value);
}

QString OctaveVariableManagementExtension::removeVariable(const QString& name)
{
    return QString::fromLatin1("clear %1;").arg(name);
}

QString OctaveVariableManagementExtension::clearVariables()
{
    return QLatin1String("clear;");
}

QString OctaveVariableManagementExtension::saveVariables(const QString& fileName)
{
    return QString::fromLatin1("save %1;").arg(fileName);
}

QString OctaveVariableManagementExtension::loadVariables(const QString& fileName)
{
    return QString::fromLatin1("load %1;").arg(fileName);
}

OCTAVE_EXT_CDTOR(Packaging)

QString OctavePackagingExtension::importPackage(const QString& package)
{
    return QString::fromLatin1("pkg load %1").arg(package);
}
