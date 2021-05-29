/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#include "pythonextensions.h"

#include <QDebug>

#include <KLocalizedString>

#include "pythonutils.h"

#include "settings.h"

#define PYTHON_EXT_CDTOR(name) Python##name##Extension::Python##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Python##name##Extension::~Python##name##Extension() {}

PYTHON_EXT_CDTOR(LinearAlgebra)

QString PythonLinearAlgebraExtension::createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type)
{
    Q_UNUSED(type);

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
    QString command;
    QString limits;

    int val = PythonSettings::plotExtenstionGraphicPackage();
    switch (val)
    {
        case PythonSettings::EnumPlotExtenstionGraphicPackage::matplotlib:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1("plt.xlim(%1, %2)\n").arg(left, right);
            command = QString::fromLatin1(
                "import matplotlib.pyplot as plt\n"
                "\n"
                "plt.plot(%1, %2)\n"
                "%3"
                "plt.show()"
            ).arg(variable, function, limits);
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::pylab:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1("pylab.xlim(%1, %2)\n").arg(left, right);
            command = QString::fromLatin1(
                "import pylab\n"
                "\n"
                "pylab.clf()\n"
                "pylab.plot(%1, %2)\n"
                "%3"
                "pylab.show()"
            ).arg(variable, function, limits);
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::plotly:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1("fig.update_layout(xaxis=dict(range=[%1, %2]))\n").arg(left, right);
            command = QString::fromLatin1(
                "import plotly.graph_objects as go\n"
                "\n"
                "fig = go.Figure(data=go.Scatter(x=%1, y=%2))\n"
                "%3"
                "fig.show()"
            ).arg(variable, function, limits);
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::gr:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1("\nmlab.xlim(%1, %2)").arg(left, right);
            command = QString::fromLatin1(
                "from gr.pygr import mlab\n"
                "\n"
                "mlab.plot(%1, %2)"
                "%3"
            ).arg(variable, function, limits);
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::bokeh:
            if (!left.isEmpty() && !right.isEmpty())
                limits = QString::fromLatin1("x_range=(%1, %2)").arg(left, right);
            command = QString::fromLatin1(
                "from bokeh.plotting import figure, show\n"
                "\n"
                "fig = figure(%3)\n"
                "fig.line(%1, %2)\n"
                "show(fig)"
            ).arg(variable, function, limits);
            break;
    };

    return command;
}

QString PythonPlotExtension::plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2)
{
    QString command;

    const Interval& interval1 = var1.second;
    const Interval& interval2 = var2.second;

    QString interval1Limits;
    QString interval2Limits;

    int val = PythonSettings::plotExtenstionGraphicPackage();
    switch (val)
    {
        case PythonSettings::EnumPlotExtenstionGraphicPackage::matplotlib:
                        if(!interval1.first.isEmpty() && !interval1.second.isEmpty()){
                interval1Limits = QString::fromLatin1("ax3D.set_xlim3d(%1, %2)\n").arg(interval1.first, interval1.second);
            }

            if(!interval2.first.isEmpty() && !interval2.second.isEmpty()){
                interval2Limits = QString::fromLatin1("ax3D.set_ylim3d(%1, %2)\n").arg(interval2.first, interval2.second);
            }

            command = QString::fromLatin1(
                "import matplotlib.pyplot as plt\n"
                "from mpl_toolkits.mplot3d import Axes3D\n\n"
                "fig3D = plt.figure()\n"
                "ax3D = fig3D.gca(projection='3d')\n"
                "ax3D.plot_surface(%1, %2, %3)\n"    \
                "%4%5"                                                             \
                "plt.show()"
            ).arg(var1.first, var2.first, function, interval1Limits, interval2Limits);
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::pylab:
            if(!interval1.first.isEmpty() && !interval1.second.isEmpty()){
                interval1Limits = QString::fromLatin1("ax3D.set_xlim3d(%1, %2)\n").arg(interval1.first, interval1.second);
            }

            if(!interval2.first.isEmpty() && !interval2.second.isEmpty()){
                interval2Limits = QString::fromLatin1("ax3D.set_ylim3d(%1, %2)\n").arg(interval2.first, interval2.second);
            }

            command = QString::fromLatin1(
                "import pylab\n"
                "from mpl_toolkits.mplot3d import Axes3D\n\n"
                "fig3D = pylab.figure()\n"
                "ax3D = fig3D.gca(projection='3d')\n"
                "ax3D.plot_surface(%1, %2, %3)\n"    \
                "%4%5"                                                             \
                "pylab.show()"
            ).arg(var1.first, var2.first, function, interval1Limits, interval2Limits);
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::plotly:
            if(!interval1.first.isEmpty() && !interval1.second.isEmpty()){
                interval1Limits = QString::fromLatin1("fig.update_layout(xaxis=dict(range=[%1, %2]))\n").arg(interval1.first, interval1.second);
            }

            if(!interval2.first.isEmpty() && !interval2.second.isEmpty()){
                interval2Limits = QString::fromLatin1("fig.update_layout(yaxis=dict(range=[%1, %2]))\n").arg(interval2.first, interval2.second);
            }

            command = QString::fromLatin1(
                "import plotly.graph_objects as go\n"
                "\n"
                "fig = go.Figure(data=go.Scatter3d(x=%1, y=%2, z=%3))\n"
                "%4%5"
                "fig.show()"
            ).arg(var1.first, var2.first, function, interval1Limits, interval2Limits);
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::gr:
            {
            if(!interval1.first.isEmpty() && !interval1.second.isEmpty()){
                interval1Limits = QString::fromLatin1("mlab.xlim(%1, %2)\n").arg(interval1.first, interval1.second);
            }

            if(!interval2.first.isEmpty() && !interval2.second.isEmpty()){
                interval2Limits = QString::fromLatin1("mlab.ylim(%1, %2)").arg(interval2.first, interval2.second);
            }
            QString newLinePlacement;
            if(!interval1Limits.isEmpty() || !interval2Limits.isEmpty())
                newLinePlacement = QLatin1String("\n");

            command = QString::fromLatin1(
                "from gr.pygr import mlab\n"
                "\n"
                "mlab.plot3(%1, %2, %3)%6"
                "%4%5"
            ).arg(var1.first, var2.first, function, interval1Limits, interval2Limits, newLinePlacement);
            }
            break;

        case PythonSettings::EnumPlotExtenstionGraphicPackage::bokeh:
            command = i18n("# Sorry, but Bokeh doesn't support 3d plotting");
            break;
    };

    return command;
}

PYTHON_EXT_CDTOR(Script)

QString PythonScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("exec(open(\"%1\").read())").arg(path);
}

QString PythonScriptExtension::scriptFileFilter()
{
    return i18n("Python script file (*.py)");
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
    return QString::fromLatin1("%1 = %2").arg(name, value);
}

QString PythonVariableManagementExtension::removeVariable(const QString& name)
{
    return QString::fromLatin1("del(%1)").arg(name);
}

QString PythonVariableManagementExtension::clearVariables()
{
    return fromSource(QLatin1String(":/py/variables_cleaner.py"));
}

QString PythonVariableManagementExtension::saveVariables(const QString& fileName)
{
    return fromSource(QLatin1String(":/py/variables_saver.py")).arg(fileName);
}

QString PythonVariableManagementExtension::loadVariables(const QString& fileName)
{
    return fromSource(QLatin1String(":/py/variables_loader.py")).arg(fileName);
}
