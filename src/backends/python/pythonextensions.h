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

#ifndef PYTHONEXTENSIONS_H
#define PYTHONEXTENSIONS_H

#include <extension.h>

#define PYTHON_EXT_CDTOR_DECL(name) Python##name##Extension(QObject* parent); \
                                     ~Python##name##Extension();

class PythonPackagingExtension : public Cantor::PackagingExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(Packaging)
    virtual QString importPackage(const QString& module);
};

class PythonVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(VariableManagement)
    virtual QString addVariable(const QString& name, const QString& value);
    virtual QString setValue(const QString& name, const QString& value);
    virtual QString removeVariable(const QString& name);
    virtual QString saveVariables(const QString& fileName);
    virtual QString loadVariables(const QString& fileName);
    virtual QString clearVariables();
};

#endif // PYTHONEXTENSIONS_H
