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


class JuliaLinearAlgebraExtension: public Cantor::LinearAlgebraExtension
{
public:
    JULIA_EXT_CDTOR_DECL(LinearAlgebra)

    virtual QString createVector(
        const QStringList &entries,
        VectorType type) override;

    virtual QString nullVector(int size, VectorType type) override;

    virtual QString createMatrix(
        const Cantor::LinearAlgebraExtension::Matrix &matrix) override;

    virtual QString identityMatrix(int size) override;
    virtual QString nullMatrix(int rows, int columns) override;
    virtual QString rank(const QString &matrix) override;
    virtual QString invertMatrix(const QString &matrix) override;
    virtual QString charPoly(const QString &matrix) override;
    virtual QString eigenVectors(const QString &matrix) override;
    virtual QString eigenValues(const QString &matrix) override;
};

class JuliaPackagingExtension: public Cantor::PackagingExtension
{
public:
    JULIA_EXT_CDTOR_DECL(Packaging)

    virtual QString importPackage(const QString &module) override;
};

class JuliaPlotExtension: public Cantor::PlotExtension
{
public:
    JULIA_EXT_CDTOR_DECL(Plot)

    virtual QString plotFunction2d(
        const QString &function,
        const QString &variable,
        const QString &left,
        const QString &right) override;

    virtual QString plotFunction3d(
        const QString &function,
        VariableParameter var1,
        VariableParameter var2) override;
};

class JuliaScriptExtension: public Cantor::ScriptExtension
{
public:
    JULIA_EXT_CDTOR_DECL(Script)

    virtual QString scriptFileFilter() override;
    virtual QString highlightingMode() override;
    virtual QString runExternalScript(const QString &path) override;
};

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
