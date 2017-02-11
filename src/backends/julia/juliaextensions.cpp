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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliaextensions.h"

#include <QDebug>
#include <KLocalizedString>

#include "juliascriptloading.h"

#define JULIA_EXT_CDTOR(name) Julia##name##Extension::Julia##name##Extension(QObject *parent) : name##Extension(parent) {} \
                              Julia##name##Extension::~Julia##name##Extension() {}


JULIA_EXT_CDTOR(LinearAlgebra)

QString JuliaLinearAlgebraExtension::createVector(
    const QStringList &entries,
    Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command;
    command += QLatin1String("[");

    QString separator = QLatin1String(type == ColumnVector ? ", " : " ");

    for (const QString &entry : entries) {
        command += entry + separator;
    }

    command.chop(separator.size());
    command += QLatin1String("]\n");

    return command;
}

QString JuliaLinearAlgebraExtension::createMatrix(
    const Cantor::LinearAlgebraExtension::Matrix &matrix)
{
    QString command;
    command += QLatin1String("[");

    for (const QStringList row : matrix) {
        for (const QString entry : row) {
            command += entry;
            command += QLatin1String(" ");
        }
        command.chop(1);
        command += QLatin1String("; ");
    }

    command.chop(2);
    command += QLatin1String("]");

    return command;
}

QString JuliaLinearAlgebraExtension::eigenValues(const QString &matrix)
{
    return QString::fromLatin1("eig(%1)[1]").arg(matrix);
}

QString JuliaLinearAlgebraExtension::eigenVectors(const QString &matrix)
{
    return QString::fromLatin1("eig(%1)[2]").arg(matrix);
}

QString JuliaLinearAlgebraExtension::identityMatrix(int size)
{
    return QString::fromLatin1("eye(%1)").arg(size);
}

QString JuliaLinearAlgebraExtension::invertMatrix(const QString &matrix)
{
    return QString::fromLatin1("inv(%1)").arg(matrix);
}

QString JuliaLinearAlgebraExtension::nullMatrix(int rows, int columns)
{
    return QString::fromLatin1("zeros(%1, %2)").arg(rows).arg(columns);
}

QString JuliaLinearAlgebraExtension::nullVector(
    int size,
    Cantor::LinearAlgebraExtension::VectorType type)
{
    switch (type) {
        case ColumnVector:
            return QString::fromLatin1("zeros(%1)").arg(size);
        case RowVector:
            return QString::fromLatin1("zeros(%1, %2)").arg(1).arg(size);
        default:
            return Cantor::LinearAlgebraExtension::nullVector(size, type);
    }
}

QString JuliaLinearAlgebraExtension::rank(const QString &matrix)
{
    return QString::fromLatin1("rank(%1)").arg(matrix);
}

QString JuliaLinearAlgebraExtension::charPoly(const QString &matrix)
{
    return QString::fromLatin1("poly(%1)").arg(matrix);
}


JULIA_EXT_CDTOR(Packaging)

QString JuliaPackagingExtension::importPackage(const QString &package)
{
    return QString::fromLatin1("import %1").arg(package);
}


JULIA_EXT_CDTOR(Plot)

QString JuliaPlotExtension::plotFunction2d(
    const QString &function,
    const QString &variable,
    const QString &left,
    const QString &right)
{
    auto new_left = left;
    auto new_right = right;
    if (new_left.isEmpty() && new_right.isEmpty()) {
        new_left = QLatin1String("-1");
        new_right = QLatin1String("1");
    } else if (new_left.isEmpty()) {
        new_left = QString::fromLatin1("(%1) - 1").arg(new_right);
    } else if (new_right.isEmpty()) {
        new_right = QString::fromLatin1("(%1) + 1").arg(new_left);
    }

    auto xlimits = QString::fromLatin1("GR.xlim((%1, %2))\n").arg(new_left).arg(new_right);
    auto linspace =
        QString::fromLatin1("linspace(%1, %2, 100)").arg(new_left).arg(new_right);

    return QString::fromLatin1(
        "import GR\n"
        "%3"
        "GR.plot(%1, [%2 for %4 in %1])\n"
    ).arg(linspace).arg(function).arg(xlimits).arg(variable);
}

QString JuliaPlotExtension::plotFunction3d(
    const QString &function,
    Cantor::PlotExtension::VariableParameter var1,
    Cantor::PlotExtension::VariableParameter var2)
{

    auto update_interval = [](Interval &interval) {
        if (interval.first.isEmpty() && interval.second.isEmpty()) {
            interval.first = QLatin1String("-1");
            interval.second = QLatin1String("1");
        } else if (interval.first.isEmpty()) {
            interval.second = QString::fromLatin1("(%1) + 1")
                .arg(interval.first);
        } else if (interval.second.isEmpty()) {
            interval.first = QString::fromLatin1("(%1) - 1")
                .arg(interval.second);
        }
    };

    Interval interval1 = var1.second;
    Interval interval2 = var2.second;

    update_interval(interval1);
    update_interval(interval2);

    return QString::fromLatin1(
        "import GR\n"
        "values = zeros(100, 100)\n"
        "for p_x in enumerate(linspace(%1, %2, 100))\n"
        "    i, %6 = p_x\n"
        "    for p_y in enumerate(linspace(%3, %4, 100))\n"
        "        j, %7 = p_y\n"
        "        values[i, j] = %5\n"
        "    end\n"
        "end\n"
        "GR.surface(linspace(%1, %2, 100), linspace(%3, %4, 100), values)\n"
    ).arg(interval1.first).arg(interval1.second)
        .arg(interval2.first).arg(interval2.second)
        .arg(function)
        .arg(var1.first).arg(var2.first);
}


JULIA_EXT_CDTOR(Script)

QString JuliaScriptExtension::runExternalScript(const QString &path)
{
    return QString::fromLatin1("include(\"%1\")").arg(path);
}

QString JuliaScriptExtension::scriptFileFilter()
{
    return i18n("Julia script file (*.jl)");
}

QString JuliaScriptExtension::highlightingMode()
{
    return QLatin1String("julia");
}


JULIA_EXT_CDTOR(VariableManagement)

const QString JuliaVariableManagementExtension::REMOVED_VARIABLE_MARKER =
    QLatin1String("__REM__");

QString JuliaVariableManagementExtension::addVariable(
    const QString &name,
    const QString &value)
{
    return setValue(name, value);
}

QString JuliaVariableManagementExtension::setValue(
    const QString &name,
    const QString &value)
{
    return QString::fromLatin1("%1 = %2").arg(name).arg(value);
}

QString JuliaVariableManagementExtension::removeVariable(const QString &name)
{
    // There is no way to completely delete object from scope:
    // http://docs.julialang.org/en/release-0.4/manual/faq/#how-do-i-delete-an-object-in-memory
    return QString::fromLatin1("%1 = \"%2\"")
        .arg(name).arg(REMOVED_VARIABLE_MARKER);
}

QString JuliaVariableManagementExtension::clearVariables()
{
    return loadScript(QLatin1String("variables_cleaner"))
        .arg(REMOVED_VARIABLE_MARKER);
}

QString JuliaVariableManagementExtension::saveVariables(const QString &fileName)
{
    return loadScript(QLatin1String("variables_saver")).arg(fileName);
}

QString JuliaVariableManagementExtension::loadVariables(const QString &fileName)
{
    return loadScript(QLatin1String("variables_loader")).arg(fileName);
}
