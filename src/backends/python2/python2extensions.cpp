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

#include "python2extensions.h"
#include <KLocale>
#include <KDebug>

#define PYTHON2_EXT_CDTOR(name) Python2##name##Extension::Python2##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Python2##name##Extension::~Python2##name##Extension() {}

PYTHON2_EXT_CDTOR(LinearAlgebra)

QString Python2LinearAlgebraExtension::createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command;
    command += "numpy.matrix([";

    foreach (const QString& entry, entries)
    {
        command += entry + ", ";
    }

    command.chop(2);
    command += "])\n";

    return command;
}

QString Python2LinearAlgebraExtension::createMatrix(const Cantor::LinearAlgebraExtension::Matrix& matrix)
{
    QString command;
    command += "numpy.matrix([[";

    foreach (const QStringList row, matrix)
    {
        foreach (const QString entry, row)
        {
            command += entry;
            command += ", ";
        }
        command.chop(2);
        command += "], [";
    }

    command.chop(3);
    command += "])";

    return command;
}

QString Python2LinearAlgebraExtension::eigenValues(const QString& matrix)
{
    return QString("numpy.linalg.eigvals(%1)").arg(matrix);
}

QString Python2LinearAlgebraExtension::eigenVectors(const QString& matrix)
{
    return QString("numpy.linalg.eig(%1)").arg(matrix);
}

QString Python2LinearAlgebraExtension::identityMatrix(int size)
{
    return QString("numpy.identity(%1)").arg(size);
}

QString Python2LinearAlgebraExtension::invertMatrix(const QString& matrix)
{
    return QString("numpy.linalg.inv(%1)").arg(matrix);
}

QString Python2LinearAlgebraExtension::nullMatrix(int rows, int columns)
{
    return QString("numpy.zeros(%1, %2)").arg(rows).arg(columns);
}

QString Python2LinearAlgebraExtension::nullVector(int size, Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command = "numpy.zeros(%1, %2)";
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

QString Python2LinearAlgebraExtension::rank(const QString& matrix)
{
    return QString("numpy.linalg.matrix_rank(%1)").arg(matrix);
}

QString Python2LinearAlgebraExtension::charPoly(const QString& matrix)
{
    return QString("numpy.poly(%1)").arg(matrix);
}

PYTHON2_EXT_CDTOR(Packaging)

QString Python2PackagingExtension::importPackage(const QString& package)
{
    return QString("import %1").arg(package);
}

PYTHON2_EXT_CDTOR(Plot)

QString Python2PlotExtension::plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right)
{
    QString argumentToPlot = variable;
    QString xlimits;

    if(!function.isEmpty()){
        argumentToPlot = function + "(" + variable + ")";
    }

    if(!left.isEmpty() && !right.isEmpty()){
        xlimits = QString("pylab.xlim(%1, %2)\n").arg(left).arg(right);
    }

    return QString("pylab.clf()\n"                     \
                   "pylab.plot(%1)\n"                  \
                   + xlimits +                         \
                   "pylab.show()").arg(argumentToPlot);
}

QString Python2PlotExtension::plotFunction3d(const QString& function, Cantor::PlotExtension::VariableParameter var1, Cantor::PlotExtension::VariableParameter var2)
{
    const Interval& interval1 = var1.second;
    const Interval& interval2 = var2.second;

    QString interval1Limits;
    QString interval2Limits;

    if(!interval1.first.isEmpty() && !interval1.second.isEmpty()){
        interval1Limits = QString("ax3D.set_xlim3d(%1, %2)\n").arg(interval1.first).arg(interval1.second);
    }

    if(!interval2.first.isEmpty() && !interval2.second.isEmpty()){
        interval2Limits = QString("ax3D.set_ylim3d(%1, %2)\n").arg(interval2.first).arg(interval2.second);
    }

    return QString("from mpl_toolkits.mplot3d import Axes3D\n\n"                      \
                   "fig3D = pylab.figure()\n"                                         \
                   "ax3D = fig3D.gca(projection='3d')\n"                              \
                   "ax3D.plot_surface(%1, %2, %3(%1, %2), rstride=4, cstride=4)\n"    \
                   + interval1Limits + interval2Limits +                              \
                   "pylab.show()").arg(var1.first).arg(var2.first).arg(function);
}

PYTHON2_EXT_CDTOR(Script)

QString Python2ScriptExtension::runExternalScript(const QString& path)
{
    return QString("execfile(\"%1\")").arg(path);
}

QString Python2ScriptExtension::scriptFileFilter()
{
    return i18n("*.py|Python script file");
}

QString Python2ScriptExtension::highlightingMode()
{
    return QString("python");
}

PYTHON2_EXT_CDTOR(VariableManagement)

QString Python2VariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name, value);
}

QString Python2VariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString("%1 = %2").arg(name).arg(value);
}

QString Python2VariableManagementExtension::removeVariable(const QString& name)
{
    return QString("del(%1)").arg(name);
}

QString Python2VariableManagementExtension::clearVariables()
{
    QString delVariablesPythonSession = "for keyPythonBackend in dir():\n"                                 \
                                        "    if (not 'PythonBackend' in keyPythonBackend)\ "               \
                                        "and (not '__' in keyPythonBackend):\n"                            \
                                        "        del(globals()[keyPythonBackend])\n"                       \
                                        "del(keyPythonBackend)\n";
    return delVariablesPythonSession;
}

QString Python2VariableManagementExtension::saveVariables(const QString& fileName)
{
    QString classSavePythonSession = "import shelve\n"                                                               \
                                     "shelvePythonBackend = shelve.open('%1', 'n')\n"                                \
                                     "for keyPythonBackend in dir():\n"                                              \
                                     "    if (not 'PythonBackend' in keyPythonBackend)\ "                            \
                                     "and (not '__' in keyPythonBackend)\ "                                          \
                                     "and (not '<module ' in unicode(globals()[keyPythonBackend])):\n"               \
                                     "        shelvePythonBackend[keyPythonBackend] = globals()[keyPythonBackend]\n" \
                                     "shelvePythonBackend.close()\n"                                                 \
                                     "del(shelve)\n"                                                                 \
                                     "del(shelvePythonBackend)\n"                                                    \
                                     "del(keyPythonBackend)\n";

    return classSavePythonSession.arg(fileName);
}

QString Python2VariableManagementExtension::loadVariables(const QString& fileName)
{
    QString classLoadPythonSession = "import shelve\n"                                                           \
                                     "shelvePythonBackend = shelve.open('%1')\n"                                 \
                                     "for keyPythonBackend in shelvePythonBackend:\n"                            \
                                     "    globals()[keyPythonBackend] = shelvePythonBackend[keyPythonBackend]\n" \
                                     "shelvePythonBackend.close()\n"                                             \
                                     "del(shelve)\n"                                                             \
                                     "del(shelvePythonBackend)\n"                                                \
                                     "del(keyPythonBackend)\n";

    return classLoadPythonSession.arg(fileName);
}
