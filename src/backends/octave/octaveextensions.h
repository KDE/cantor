/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#ifndef OCTAVEEXTENSIONS_H
#define OCTAVEEXTENSIONS_H

#include <extension.h>

#define OCTAVE_EXT_CDTOR_DECL(name) Octave##name##Extension(QObject* parent); \
                                     ~Octave##name##Extension();

class OctaveHistoryExtension : public Cantor::HistoryExtension
{
    public:
        OCTAVE_EXT_CDTOR_DECL(History)
        QString lastResult() override;
};

class OctaveScriptExtension : public Cantor::ScriptExtension
{
    public:
        OCTAVE_EXT_CDTOR_DECL(Script)
    QString scriptFileFilter() override;
    QString highlightingMode() override;
    QString runExternalScript(const QString& path) override;
    QString commandSeparator() override;
};

class OctavePlotExtension : public Cantor::PlotExtension
{
    public:
    OCTAVE_EXT_CDTOR_DECL(Plot)
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) override;
    QString plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2) override;
};

class OctaveLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
    public:
    OCTAVE_EXT_CDTOR_DECL(LinearAlgebra)
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

class OctaveVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    OCTAVE_EXT_CDTOR_DECL(VariableManagement)
    QString addVariable(const QString& name, const QString& value) override;
    QString setValue(const QString& name, const QString& value) override;
    QString removeVariable(const QString& name) override;
    QString saveVariables(const QString& fileName) override;
    QString loadVariables(const QString& fileName) override;
    QString clearVariables() override;
};

class OctavePackagingExtension : public Cantor::PackagingExtension
{
    public:
    OCTAVE_EXT_CDTOR_DECL(Packaging)
    QString importPackage(const QString& package) override;
};

#endif // OCTAVEEXTENSIONS_H
