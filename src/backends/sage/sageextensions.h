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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _SAGEEXTENSIONS_H
#define _SAGEEXTENSIONS_H

#include "extension.h"

class SageHistoryExtension : public Cantor::HistoryExtension
{
  public:
    explicit SageHistoryExtension( QObject* parent );
    ~SageHistoryExtension() override;
  public Q_SLOTS:
    QString lastResult() override;
};

class SageScriptExtension : public Cantor::ScriptExtension
{
  public:
    explicit SageScriptExtension(QObject* parent);
    ~SageScriptExtension() override;
  public Q_SLOTS:
    QString runExternalScript(const QString& path) override;
    QString scriptFileFilter() override;
    QString highlightingMode() override;
};

class SageCASExtension : public Cantor::CASExtension
{
  public:
    explicit SageCASExtension( QObject* parent);
    ~SageCASExtension() override;

  public Q_SLOTS:
    QString solve(const QStringList& equations, const QStringList& variables) override;
    QString simplify(const QString& expression) override;
    QString expand(const QString& expression) override;

};

class SageCalculusExtension : public Cantor::CalculusExtension
{
  public:
    explicit SageCalculusExtension( QObject* parent);
    ~SageCalculusExtension() override;

  public Q_SLOTS:
    QString limit(const QString& expression, const QString& variable, const QString& limit) override;
    QString differentiate(const QString& function,const QString& variable, int times) override;
    QString integrate(const QString& function, const QString& variable) override;
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right) override;
};

class SageLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    explicit SageLinearAlgebraExtension( QObject* parent);
    ~SageLinearAlgebraExtension() override;

  public Q_SLOTS:
    //Commands to create Vectors/Matrices
    QString createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type) override;
    QString nullVector(int size, Cantor::LinearAlgebraExtension::VectorType type) override;
    QString createMatrix(const Matrix& matrix) override;
    QString identityMatrix(int size) override;
    QString nullMatrix(int rows,int columns) override;

    //basic functions
    QString rank(const QString& matrix) override;
    QString invertMatrix(const QString& matrix) override;
    QString charPoly(const QString& matrix) override;
    QString eigenVectors(const QString& matrix) override;
    QString eigenValues(const QString& matrix) override;
};

class SagePlotExtension : public Cantor::PlotExtension
{
  public:
    explicit SagePlotExtension(QObject* parent);
    ~SagePlotExtension() override;
  public Q_SLOTS:
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) override;
    QString plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2) override;
};

class SagePackagingExtension : public Cantor::PackagingExtension
{
  public:
    explicit SagePackagingExtension(QObject* parent);
    ~SagePackagingExtension() override;

  public Q_SLOTS:
    QString importPackage(const QString& module) override;
};

#endif /* _SAGEEXTENSIONS_H */
