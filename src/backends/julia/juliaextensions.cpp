/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#include "juliaextensions.h"

#include <QDebug>
#include <KLocalizedString>

#include "settings.h"
#include "juliascriptloading.h"

#define JULIA_EXT_CDTOR(name) Julia##name##Extension::Julia##name##Extension(QObject *parent) : name##Extension(parent) {} \
                              Julia##name##Extension::~Julia##name##Extension() {}

JULIA_EXT_CDTOR(LinearAlgebra)

QString JuliaLinearAlgebraExtension::createVector(
    const QStringList &entries,
    Cantor::LinearAlgebraExtension::VectorType type)
{
    QString command = QLatin1String("[");
    QString separator = QLatin1String(type == ColumnVector ? ", " : " ");

    for (const auto& entry : entries) {
        command += entry + separator;
    }

    command.chop(separator.size());
    command += QLatin1String("]\n");

    return command;
}

QString JuliaLinearAlgebraExtension::createMatrix(
    const Cantor::LinearAlgebraExtension::Matrix &matrix)
{
    QString command = QLatin1String("[");

    for (const auto& row : matrix) {
        for (const auto& entry : row) {
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
    QString command;
    QString limits;

    switch(JuliaSettings::plotExtenstionGraphicPackage())
    {
        case JuliaSettings::EnumPlotExtenstionGraphicPackage::gr:
        {
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1("GR.xlim((%1, %2))\n").arg(left).arg(right);

            command = QString::fromLatin1(
                "import GR\n"
                "\n"
                "%3"
                "GR.plot(%1, %2)"
            ).arg(variable).arg(function).arg(limits);
            break;
        }

        case JuliaSettings::EnumPlotExtenstionGraphicPackage::plots:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1(", xlims = (%1, %2)").arg(left).arg(right);

            command = QString::fromLatin1(
                "import Plots\n"
                "\n"
                "Plots.plot(%1, %2%3)"
            ).arg(variable, function, limits);
            break;

        case JuliaSettings::EnumPlotExtenstionGraphicPackage::pyplot:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1("PyPlot.xlim(%1, %2)\n").arg(left).arg(right);

            command = QString::fromLatin1(
                "import PyPlot\n"
                "\n"
                "%3"
                "PyPlot.plot(%1, %2)"
            ).arg(variable, function, limits);
            break;

        case JuliaSettings::EnumPlotExtenstionGraphicPackage::gadfly:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1(", Gadfly.Scale.x_continuous(minvalue=%1, maxvalue=%2)").arg(left).arg(right);

            command = QString::fromLatin1(
                "import Gadfly\n"
                "\n"
                "Gadfly.plot(x=%1, y=%2%3)"
            ).arg(variable, function, limits);
            break;
    }

    return command;
}

QString JuliaPlotExtension::plotFunction3d(
    const QString &function,
    const VariableParameter& var1,
    const VariableParameter& var2)
{
    QString command;

    const Interval& interval1 = var1.second;
    const Interval& interval2 = var2.second;

    QString interval1Limits;
    QString interval2Limits;

    switch(JuliaSettings::plotExtenstionGraphicPackage())
    {
        case JuliaSettings::EnumPlotExtenstionGraphicPackage::gr:
        {
            if (!interval1.first.isEmpty() && !interval1.second.isEmpty())
                interval1Limits = QString::fromLatin1("GR.xlim((%1, %2))\n").arg(interval1.first).arg(interval1.second);

            if (!interval2.first.isEmpty() && !interval2.second.isEmpty())
                interval1Limits = QString::fromLatin1("GR.ylim((%1, %2))\n").arg(interval2.first).arg(interval2.second);

            command = QString::fromLatin1(
                "import GR\n"
                "\n"
                "%4%5"
                "GR.plot3(%1, %2, %3)"
            ).arg(interval1.first, interval2.first, function, interval1Limits, interval2Limits);
            break;
        }

        case JuliaSettings::EnumPlotExtenstionGraphicPackage::plots:
            if (!interval1.first.isEmpty() && !interval1.second.isEmpty())
                interval1Limits = QString::fromLatin1(", xlims = (%1, %2)").arg(interval1.first).arg(interval1.second);

            if (!interval2.first.isEmpty() && !interval2.second.isEmpty())
                interval1Limits = QString::fromLatin1(", ylims = (%1, %2)").arg(interval2.first).arg(interval2.second);

            command = QString::fromLatin1(
                "import Plots\n"
                "\n"
                "%4%5"
                "GR.plot3d(%1, %2, %3)"
            ).arg(interval1.first, interval2.first, function, interval1Limits, interval2Limits);
            break;

        case JuliaSettings::EnumPlotExtenstionGraphicPackage::pyplot:
            if (!interval1.first.isEmpty() && !interval1.second.isEmpty())
                interval1Limits = QString::fromLatin1("GR.xlim((%1, %2))\n").arg(interval1.first).arg(interval1.second);

            if (!interval2.first.isEmpty() && !interval2.second.isEmpty())
                interval1Limits = QString::fromLatin1("GR.ylim((%1, %2))\n").arg(interval2.first).arg(interval2.second);

            command = QString::fromLatin1(
                "import GR\n"
                "\n"
                "%4%5"
                "PyPlot.plot3D(%1, %2, %3)"
            ).arg(interval1.first, interval2.first, function, interval1Limits, interval2Limits);
            break;

        case JuliaSettings::EnumPlotExtenstionGraphicPackage::gadfly:
            command = i18n("# Sorry, but Gadfly don't support 3d plots");
            break;
    }

    return command;
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
    // https://docs.julialang.org/en/v1/manual/faq/#How-do-I-delete-an-object-in-memory?-1
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
