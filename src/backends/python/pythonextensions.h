/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#ifndef PYTHONEXTENSIONS_H
#define PYTHONEXTENSIONS_H

#include <extension.h>

#define PYTHON_EXT_CDTOR_DECL(name) Python##name##Extension(QObject* parent); \
                                    ~Python##name##Extension();

class PythonLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(LinearAlgebra)
    QString createVector(const QStringList& entries, VectorType type) override;
    QString nullVector(int size, VectorType type) override;
    QString createMatrix(const Cantor::LinearAlgebraExtension::Matrix& matrix) override;
    QString identityMatrix(int size) override;
    QString nullMatrix(int rows, int columns) override;
    QString rank(const QString& matrix) override;
    QString invertMatrix(const QString& matrix) override;
    QString charPoly(const QString& matrix) override;
    QString eigenVectors(const QString& matrix) override;
    QString eigenValues(const QString& matrix) override;
};

class PythonPackagingExtension : public Cantor::PackagingExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(Packaging)
    QString importPackage(const QString& module) override;
};

class PythonPlotExtension : public Cantor::PlotExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(Plot)
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) override;
    QString plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2) override;
};

class PythonScriptExtension : public Cantor::ScriptExtension
{
    public:
        PYTHON_EXT_CDTOR_DECL(Script)
        QString scriptFileFilter() override;
        QString highlightingMode() override;
        QString runExternalScript(const QString& path) override;
};

class PythonVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(VariableManagement)
    QString addVariable(const QString& name, const QString& value) override;
    QString setValue(const QString& name, const QString& value) override;
    QString removeVariable(const QString& name) override;
    QString saveVariables(const QString& fileName) override;
    QString loadVariables(const QString& fileName) override;
    QString clearVariables() override;
};

#endif // PYTHONEXTENSIONS_H
