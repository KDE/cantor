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
