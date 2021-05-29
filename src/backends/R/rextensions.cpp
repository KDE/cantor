/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "rextensions.h"

#include <KLocalizedString>

RScriptExtension::RScriptExtension(QObject* parent) : Cantor::ScriptExtension(parent)
{

}

QString RScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("source(\"%1\")").arg(path);
}

QString RScriptExtension::scriptFileFilter()
{
    return i18n("R script file (*.R)");
}

QString RScriptExtension::highlightingMode()
{
    return QLatin1String("r script");
}

RPlotExtension::RPlotExtension(QObject* parent) : Cantor::AdvancedPlotExtension(parent)
{
}
// TODO: injection prevention
QString RPlotExtension::accept(const Cantor::PlotTitleDirective& directive) const
{
    return QLatin1String("main=\"")+directive.title()+QLatin1String("\"");
}

QString RPlotExtension::accept(const Cantor::OrdinateScaleDirective& directive) const
{
    return QLatin1String("ylim=range(")+QString::number(directive.min())+QLatin1String(",")+QString::number(directive.max())+QLatin1String(")");
}

QString RPlotExtension::accept(const Cantor::AbscissScaleDirective& directive) const
{
    return QLatin1String("xlim=range(")+QString::number(directive.min())+QLatin1String(",")+QString::number(directive.max())+QLatin1String(")");
}

RVariableManagementExtension::RVariableManagementExtension(QObject* parent) : Cantor::VariableManagementExtension(parent)
{

}

QString RVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
        return setValue(name, value);
}

QString RVariableManagementExtension::setValue(const QString& name, const QString& value)
{
        return QString::fromLatin1("%1 = %2").arg(name, value);
}

QString RVariableManagementExtension::removeVariable(const QString& name)
{
        return QString::fromLatin1("remove(%1)").arg(name);
}

QString RVariableManagementExtension::saveVariables(const QString& fileName)
{
        Q_UNUSED(fileName);
        return QString();
}

QString RVariableManagementExtension::loadVariables(const QString& fileName)
{
        Q_UNUSED(fileName);
        return QString();
}

QString RVariableManagementExtension::clearVariables()
{
        return QLatin1String("rm(list=ls())");
}
