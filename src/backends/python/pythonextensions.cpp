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
#include <KLocale>
#include <KDebug>

#define PYTHON_EXT_CDTOR(name) Python##name##Extension::Python##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                     Python##name##Extension::~Python##name##Extension() {}

PYTHON_EXT_CDTOR(Packaging)

QString PythonPackagingExtension::importPackage(const QString& package)
{
    return QString("import %1").arg(package);
}

PYTHON_EXT_CDTOR(VariableManagement)

QString PythonVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name,value);
}

QString PythonVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString("%1 = %2").arg(name).arg(value);
}

QString PythonVariableManagementExtension::removeVariable(const QString& name)
{
    return QString("clear %1;").arg(name);
}

QString PythonVariableManagementExtension::clearVariables()
{
    return QString("clear;");
}

QString PythonVariableManagementExtension::saveVariables(const QString& fileName)
{
    return QString("save %1;").arg(fileName);
}

QString PythonVariableManagementExtension::loadVariables(const QString& fileName)
{
    return QString("load %1;").arg(fileName);
}
