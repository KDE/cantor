/************************************************************************************
*  Copyright (C) 2011 by Matteo Agostinelli <agostinelli@gmail.com>                 *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "qalculateextensions.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Variable.h>

#define QALCULATE_EXT_CDTOR(name) Qalculate##name##Extension::Qalculate##name##Extension(QObject* parent) : name##Extension(parent) {} \
                                  Qalculate##name##Extension::~Qalculate##name##Extension() {}

QALCULATE_EXT_CDTOR(History)
QALCULATE_EXT_CDTOR(VariableManagement)

QString QalculateHistoryExtension::lastResult()
{
    return "ans";
}

QString QalculateVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    return setValue(name,value);
}

QString QalculateVariableManagementExtension::setValue(const QString& name, const QString& value)
{
    return QString("%1 := %2").arg(name).arg(value);
}

QString QalculateVariableManagementExtension::removeVariable(const QString& name)
{
    CALCULATOR->getVariable(name.toStdString())->setActive(false);
}

QString QalculateVariableManagementExtension::clearVariables()
{
    CALCULATOR->resetVariables();
    return QString();
}

QString QalculateVariableManagementExtension::saveVariables(const QString& fileName)
{
    // not supported
    return QString();
}

QString QalculateVariableManagementExtension::loadVariables(const QString& fileName)
{
    // not supported
    return QString();
}
