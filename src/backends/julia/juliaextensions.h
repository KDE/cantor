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


/**
 * Implementation of linear algebra wizards for Julia
 */
class JuliaLinearAlgebraExtension: public Cantor::LinearAlgebraExtension
{
public:
    JULIA_EXT_CDTOR_DECL(LinearAlgebra)

    /**
     * @see Cantor::LinearAlgebraExtension::createVector
     */
    virtual QString createVector(
        const QStringList &entries,
        VectorType type) override;

    /**
     * @see Cantor::LinearAlgebraExtension::nullVector
     */
    virtual QString nullVector(int size, VectorType type) override;

    /**
     * @see Cantor::LinearAlgebraExtension::createMatrix
     */
    virtual QString createMatrix(
        const Cantor::LinearAlgebraExtension::Matrix &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::identityMatrix
     */
    virtual QString identityMatrix(int size) override;

    /**
     * @see Cantor::LinearAlgebraExtension::nullMatrix
     */
    virtual QString nullMatrix(int rows, int columns) override;

    /**
     * @see Cantor::LinearAlgebraExtension::rank
     */
    virtual QString rank(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::invertMatrix
     */
    virtual QString invertMatrix(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::charPoly
     */
    virtual QString charPoly(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::eigenVectors
     */
    virtual QString eigenVectors(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::eigenValues
     */
    virtual QString eigenValues(const QString &matrix) override;
};

/**
 * Implementation of packaging wizards for Julia
 */
class JuliaPackagingExtension: public Cantor::PackagingExtension
{
public:
    JULIA_EXT_CDTOR_DECL(Packaging)

    /**
     * @see Cantor::PackagingExtension::importPackage
     */
    virtual QString importPackage(const QString &module) override;
};

/**
 * Implementation of plot wizards for Julia
 *
 * Plotting is based on GR package
 */
class JuliaPlotExtension: public Cantor::PlotExtension
{
public:
    JULIA_EXT_CDTOR_DECL(Plot)

    /**
     * @see Cantor::PlotExtension::plotFunction2d
     */
    virtual QString plotFunction2d(
        const QString &function,
        const QString &variable,
        const QString &left,
        const QString &right) override;

    /**
     * @see Cantor::PlotExtension::plotFunction3d
     */
    virtual QString plotFunction3d(
        const QString &function,
        const VariableParameter& var1,
        const VariableParameter& var2) override;
};

/**
 * Implementation of script wizard for Julia
 */
class JuliaScriptExtension: public Cantor::ScriptExtension
{
public:
    JULIA_EXT_CDTOR_DECL(Script)

    /**
     * @see Cantor::ScriptExtension::scriptFileFilter
     */
    virtual QString scriptFileFilter() override;

    /**
     * @see Cantor::ScriptExtension::highlightingMode
     */
    virtual QString highlightingMode() override;

    /**
     * @see Cantor::ScriptExtension::runExternalScript
     */
    virtual QString runExternalScript(const QString &path) override;
};

/**
 * Julia variable management extension
 *
 * Based on JLD package for loading/saving variables
 */
class JuliaVariableManagementExtension: public Cantor::VariableManagementExtension
{
public:
    JULIA_EXT_CDTOR_DECL(VariableManagement)

    // There is no way to completely delete object from scope:
    // http://docs.julialang.org/en/release-0.4/manual/faq/#how-do-i-delete-an-object-in-memory
    // So we are saving special marker to variable to mark it as removed
    static const QString REMOVED_VARIABLE_MARKER;

    /**
     * @see Cantor::VariableManagementExtension::addVariable
     */
    virtual QString addVariable(
        const QString &name,
        const QString &value) override;

    /**
     * @see Cantor::VariableManagementExtension::setValue
     */
    virtual QString setValue(
        const QString &name,
        const QString &value) override;

    /**
     * @see Cantor::VariableManagementExtension::removeVariable
     */
    virtual QString removeVariable(const QString &name) override;

    /**
     * @see Cantor::VariableManagementExtension::saveVariables
     */
    virtual QString saveVariables(const QString &fileName) override;

    /**
     * @see Cantor::VariableManagementExtension::loadVariables
     */
    virtual QString loadVariables(const QString &fileName) override;

    /**
     * @see Cantor::VariableManagementExtension::clearVariables
     */
    virtual QString clearVariables() override;
};
