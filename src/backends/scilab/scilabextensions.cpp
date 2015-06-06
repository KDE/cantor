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

#include "scilabextensions.h"
#include <KLocale>

#include <QDebug>

#define SCILAB_EXT_CDTOR(name) Scilab##name##Extension::Scilab##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Scilab##name##Extension::~Scilab##name##Extension() {}

SCILAB_EXT_CDTOR(Script)

QString ScilabScriptExtension::runExternalScript(const QString& path)
{
    return QString::fromLatin1("exec(\"%1\", -1)").arg(path);
}

QString ScilabScriptExtension::scriptFileFilter()
{
    return i18n("Scilab script file (*.sce);;Scilab function file (*.sci)");
}

QString ScilabScriptExtension::highlightingMode()
{
    return QLatin1String("scilab");
}

QString ScilabScriptExtension::commandSeparator()
{
    return QLatin1String(";");
}

SCILAB_EXT_CDTOR(VariableManagement)

QString ScilabVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name,value);
}

QString ScilabVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString::fromLatin1("%1 = %2").arg(name).arg(value);
}

QString ScilabVariableManagementExtension::removeVariable(const QString& name)
{
    return QString::fromLatin1("clear %1;").arg(name);
}

QString ScilabVariableManagementExtension::clearVariables()
{
    return QLatin1String("clear;");
}

QString ScilabVariableManagementExtension::saveVariables(const QString& fileName)
{
    return QString::fromLatin1("save('%1');").arg(fileName);
}

QString ScilabVariableManagementExtension::loadVariables(const QString& fileName)
{
    return QString::fromLatin1("load('%1');").arg(fileName);
}
