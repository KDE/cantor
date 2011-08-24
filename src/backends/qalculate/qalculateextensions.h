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

#ifndef QALCULATEEXTENSIONS_H
#define QALCULATEEXTENSIONS_H

#include <extension.h>

#define QALCULATE_EXT_CDTOR_DECL(name) Qalculate##name##Extension(QObject* parent); \
                                       ~Qalculate##name##Extension();

class QalculateHistoryExtension : public Cantor::HistoryExtension
{
public:
    QALCULATE_EXT_CDTOR_DECL(History)
    virtual QString lastResult();
};

class QalculateVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    QALCULATE_EXT_CDTOR_DECL(VariableManagement)
    virtual QString addVariable(const QString& name, const QString& value);
    virtual QString setValue(const QString& name, const QString& value);
    virtual QString removeVariable(const QString& name);
    virtual QString saveVariables(const QString& fileName);
    virtual QString loadVariables(const QString& fileName);
    virtual QString clearVariables();
};

#endif // QALCULATEEXTENSIONS_H
