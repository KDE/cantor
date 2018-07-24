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
    SageHistoryExtension( QObject* parent );
    ~SageHistoryExtension() override;
  public Q_SLOTS:
    QString lastResult() Q_DECL_OVERRIDE;
};

class SageScriptExtension : public Cantor::ScriptExtension
{
  public:
    SageScriptExtension(QObject* parent);
    ~SageScriptExtension() override;
  public Q_SLOTS:
    QString runExternalScript(const QString& path) Q_DECL_OVERRIDE;
    QString scriptFileFilter() Q_DECL_OVERRIDE;
    QString highlightingMode() Q_DECL_OVERRIDE;
};

class SageCASExtension : public Cantor::CASExtension
{
  public:
    SageCASExtension( QObject* parent);
    ~SageCASExtension() override;

  public Q_SLOTS:
    QString solve(const QStringList& equations, const QStringList& variables) Q_DECL_OVERRIDE;
    QString simplify(const QString& expression) Q_DECL_OVERRIDE;
    QString expand(const QString& expression) Q_DECL_OVERRIDE;

};

class SageCalculusExtension : public Cantor::CalculusExtension
{
  public:
    SageCalculusExtension( QObject* parent);
    ~SageCalculusExtension() override;

  public Q_SLOTS:
    QString limit(const QString& expression, const QString& variable, const QString& limit) Q_DECL_OVERRIDE;
    QString differentiate(const QString& function,const QString& variable, int times) Q_DECL_OVERRIDE;
    QString integrate(const QString& function, const QString& variable) Q_DECL_OVERRIDE;
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right) Q_DECL_OVERRIDE;
};

class SageLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    SageLinearAlgebraExtension( QObject* parent);
    ~SageLinearAlgebraExtension() override;

  public Q_SLOTS:
    //Commands to create Vectors/Matrices
    QString createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type) Q_DECL_OVERRIDE;
    QString nullVector(int size, Cantor::LinearAlgebraExtension::VectorType type) Q_DECL_OVERRIDE;
    QString createMatrix(const Matrix& matrix) Q_DECL_OVERRIDE;
    QString identityMatrix(int size) Q_DECL_OVERRIDE;
    QString nullMatrix(int rows,int columns) Q_DECL_OVERRIDE;

    //basic functions
    QString rank(const QString& matrix) Q_DECL_OVERRIDE;
    QString invertMatrix(const QString& matrix) Q_DECL_OVERRIDE;
    QString charPoly(const QString& matrix) Q_DECL_OVERRIDE;
    QString eigenVectors(const QString& matrix) Q_DECL_OVERRIDE;
    QString eigenValues(const QString& matrix) Q_DECL_OVERRIDE;
};

class SagePlotExtension : public Cantor::PlotExtension
{
  public:
    SagePlotExtension(QObject* parent);
    ~SagePlotExtension() override;
  public Q_SLOTS:
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) Q_DECL_OVERRIDE;
    QString plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2) Q_DECL_OVERRIDE;
};

class SagePackagingExtension : public Cantor::PackagingExtension
{
  public:
    SagePackagingExtension(QObject* parent);
    ~SagePackagingExtension() override;

  public Q_SLOTS:
    QString importPackage(const QString& module) Q_DECL_OVERRIDE;
};

#endif /* _SAGEEXTENSIONS_H */
