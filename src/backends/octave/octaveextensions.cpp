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
#include <KDebug>

#define OCTAVE_EXT_CDTOR(name) Octave##name##Extension::Octave##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Octave##name##Extension::~Octave##name##Extension() {}


const QList<QChar> octaveMatrixOperators = QList<QChar>() << '*' << '/' << '^';
const QString octavePrintFileCommand = "print('-depsc',strcat(tempname(tempdir,'c-ob-'),'.eps')); ";

OCTAVE_EXT_CDTOR(History)

QString OctaveHistoryExtension::lastResult()
{
    return "ans";
}

OCTAVE_EXT_CDTOR(Script)

QString OctaveScriptExtension::runExternalScript(const QString& path)
{
    return QString("source \"%1\"").arg(path);
}

QString OctaveScriptExtension::scriptFileFilter()
{
    return i18n("*.m|Octave script file");
}

QString OctaveScriptExtension::languageHighlighting()
{
    return QString("octave");
}

QString OctaveScriptExtension::commandSeparator()
{
    return QString(';');
}

OCTAVE_EXT_CDTOR(Plot)

QString OctavePlotExtension::plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right)
{
    return QString("cantor_plot2d('%1','%2',%3,%4);")
		    .arg(function)
		    .arg(variable)
		    .arg(left)
		    .arg(right);
}

QString OctavePlotExtension::plotFunction3d(const QString& function, Cantor::PlotExtension::VariableParameter var1, Cantor::PlotExtension::VariableParameter var2)
{
  return QString("cantor_plot3d('%1','%2',%3,%4,'%5',%6,%7);")
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
    return QString("poly(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::createMatrix(const Cantor::LinearAlgebraExtension::Matrix& matrix)
{
    QString command;
    command += '[';
    foreach (const QStringList row, matrix)
    {
        foreach (const QString entry, row)
        {
            command += entry;
            command += ", ";
        }
        command.chop(2); // Removes the last comma
        command += "; ";
    }
    command.chop(2); // Removes the last semicolon
    command += ']';
    return command;
}

QString OctaveLinearAlgebraExtension::createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString separator;
    if (type == ColumnVector)
    {
        separator = "; ";
    }
    else
    {
        separator = ", ";
    }
    QString command;
    command += '[';
    foreach (const QString& entry, entries)
    {
        command += entry;
        command += separator;
    }
    command.chop(1);
    command += ']';
    return command;
}

QString OctaveLinearAlgebraExtension::eigenValues(const QString& matrix)
{
    return QString("eig(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    return QString("cantor_eigenvectors(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::identityMatrix(int size)
{
    return QString("eye(%1)").arg(size);
}

QString OctaveLinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString("inv(%1)").arg(matrix);
}

QString OctaveLinearAlgebraExtension::nullMatrix(int rows, int columns)
{
    return QString("zeros(%1,%2)").arg(rows).arg(columns);
}

QString OctaveLinearAlgebraExtension::nullVector(int size, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command = "zeros(%1,%2)";
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
    return QString("rank(%1)").arg(matrix);
}

OCTAVE_EXT_CDTOR(VariableManagement)

QString OctaveVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name,value);
}

QString OctaveVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString("%1 = %2").arg(name).arg(value);
}

QString OctaveVariableManagementExtension::removeVariable(const QString& name)
{
    return QString("clear %1;").arg(name);
}

QString OctaveVariableManagementExtension::clearVariables()
{
    return QString("clear;");
}

QString OctaveVariableManagementExtension::saveVariables(const QString& fileName)
{
    return QString("save %1;").arg(fileName);
}

QString OctaveVariableManagementExtension::loadVariables(const QString& fileName)
{
    return QString("load %1;").arg(fileName);
}










