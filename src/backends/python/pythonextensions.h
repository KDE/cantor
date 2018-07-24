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

class PythonLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(LinearAlgebra)
    QString createVector(const QStringList& entries, VectorType type) Q_DECL_OVERRIDE;
    QString nullVector(int size, VectorType type) Q_DECL_OVERRIDE;
    QString createMatrix(const Cantor::LinearAlgebraExtension::Matrix& matrix) Q_DECL_OVERRIDE;
    QString identityMatrix(int size) Q_DECL_OVERRIDE;
    QString nullMatrix(int rows, int columns) Q_DECL_OVERRIDE;
    QString rank(const QString& matrix) Q_DECL_OVERRIDE;
    QString invertMatrix(const QString& matrix) Q_DECL_OVERRIDE;
    QString charPoly(const QString& matrix) Q_DECL_OVERRIDE;
    QString eigenVectors(const QString& matrix) Q_DECL_OVERRIDE;
    QString eigenValues(const QString& matrix) Q_DECL_OVERRIDE;
};

class PythonPackagingExtension : public Cantor::PackagingExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(Packaging)
    QString importPackage(const QString& module) Q_DECL_OVERRIDE;
};

class PythonPlotExtension : public Cantor::PlotExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(Plot)
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) Q_DECL_OVERRIDE;
    QString plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2) Q_DECL_OVERRIDE;
};

class PythonScriptExtension : public Cantor::ScriptExtension
{
    public:
        PYTHON_EXT_CDTOR_DECL(Script)
        QString scriptFileFilter() Q_DECL_OVERRIDE;
        QString highlightingMode() Q_DECL_OVERRIDE;
        QString runExternalScript(const QString& path) Q_DECL_OVERRIDE;
};

class PythonVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    PYTHON_EXT_CDTOR_DECL(VariableManagement)
    QString addVariable(const QString& name, const QString& value) Q_DECL_OVERRIDE;
    QString setValue(const QString& name, const QString& value) Q_DECL_OVERRIDE;
    QString removeVariable(const QString& name) Q_DECL_OVERRIDE;
    QString saveVariables(const QString& fileName) Q_DECL_OVERRIDE;
    QString loadVariables(const QString& fileName) Q_DECL_OVERRIDE;
    QString clearVariables() Q_DECL_OVERRIDE;
};

#endif // PYTHONEXTENSIONS_H
