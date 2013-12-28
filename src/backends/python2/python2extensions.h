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

#ifndef PYTHON2EXTENSIONS_H
#define PYTHON2EXTENSIONS_H

#include <extension.h>

#define PYTHON2_EXT_CDTOR_DECL(name) Python2##name##Extension(QObject* parent); \
                                     ~Python2##name##Extension();

class Python2PackagingExtension : public Cantor::PackagingExtension
{
    public:
    PYTHON2_EXT_CDTOR_DECL(Packaging)
    virtual QString importPackage(const QString& module);
};

class Python2ScriptExtension : public Cantor::ScriptExtension
{
    public:
        PYTHON2_EXT_CDTOR_DECL(Script)
        virtual QString scriptFileFilter();
        virtual QString highlightingMode();
        virtual QString runExternalScript(const QString& path);
};

class Python2VariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    PYTHON2_EXT_CDTOR_DECL(VariableManagement)
    virtual QString addVariable(const QString& name, const QString& value);
    virtual QString setValue(const QString& name, const QString& value);
    virtual QString removeVariable(const QString& name);
    virtual QString saveVariables(const QString& fileName);
    virtual QString loadVariables(const QString& fileName);
    virtual QString clearVariables();
};

#endif // PYTHON2EXTENSIONS_H
