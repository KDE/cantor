/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
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
    QString createVector(
        const QStringList &entries,
        VectorType type) override;

    /**
     * @see Cantor::LinearAlgebraExtension::nullVector
     */
    QString nullVector(int size, VectorType type) override;

    /**
     * @see Cantor::LinearAlgebraExtension::createMatrix
     */
    QString createMatrix(
        const Cantor::LinearAlgebraExtension::Matrix &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::identityMatrix
     */
    QString identityMatrix(int size) override;

    /**
     * @see Cantor::LinearAlgebraExtension::nullMatrix
     */
    QString nullMatrix(int rows, int columns) override;

    /**
     * @see Cantor::LinearAlgebraExtension::rank
     */
    QString rank(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::invertMatrix
     */
    QString invertMatrix(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::charPoly
     */
    QString charPoly(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::eigenVectors
     */
    QString eigenVectors(const QString &matrix) override;

    /**
     * @see Cantor::LinearAlgebraExtension::eigenValues
     */
    QString eigenValues(const QString &matrix) override;
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
    QString importPackage(const QString &module) override;
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
    QString plotFunction2d(
        const QString &function,
        const QString &variable,
        const QString &left,
        const QString &right) override;

    /**
     * @see Cantor::PlotExtension::plotFunction3d
     */
    QString plotFunction3d(
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
    QString scriptFileFilter() override;

    /**
     * @see Cantor::ScriptExtension::highlightingMode
     */
    QString highlightingMode() override;

    /**
     * @see Cantor::ScriptExtension::runExternalScript
     */
    QString runExternalScript(const QString &path) override;
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
    // https://docs.julialang.org/en/v1/manual/faq/#How-do-I-delete-an-object-in-memory?-1
    // So we are saving special marker to variable to mark it as removed
    static const QString REMOVED_VARIABLE_MARKER;

    /**
     * @see Cantor::VariableManagementExtension::addVariable
     */
    QString addVariable(
        const QString &name,
        const QString &value) override;

    /**
     * @see Cantor::VariableManagementExtension::setValue
     */
    QString setValue(
        const QString &name,
        const QString &value) override;

    /**
     * @see Cantor::VariableManagementExtension::removeVariable
     */
    QString removeVariable(const QString &name) override;

    /**
     * @see Cantor::VariableManagementExtension::saveVariables
     */
    QString saveVariables(const QString &fileName) override;

    /**
     * @see Cantor::VariableManagementExtension::loadVariables
     */
    QString loadVariables(const QString &fileName) override;

    /**
     * @see Cantor::VariableManagementExtension::clearVariables
     */
    QString clearVariables() override;
};
