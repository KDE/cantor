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
#pragma once

#include <extension.h>

#define JULIA_EXT_CDTOR_DECL(name) Julia##name##Extension(QObject *parent); \
                                   ~Julia##name##Extension();


class JuliaVariableManagementExtension: public Cantor::VariableManagementExtension
{
public:
    JULIA_EXT_CDTOR_DECL(VariableManagement)

    static const QString REMOVED_VARIABLE_MARKER;

    virtual QString addVariable(
        const QString &name,
        const QString &value) override;

    virtual QString setValue(
        const QString &name,
        const QString &value) override;

    virtual QString removeVariable(const QString &name) override;
    virtual QString saveVariables(const QString &fileName) override;
    virtual QString loadVariables(const QString &fileName) override;
    virtual QString clearVariables() override;
};
