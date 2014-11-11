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
    ~SageHistoryExtension();
  public Q_SLOTS:
    QString lastResult();
};

class SageScriptExtension : public Cantor::ScriptExtension
{
  public:
    SageScriptExtension(QObject* parent);
    ~SageScriptExtension();
  public Q_SLOTS:
    virtual QString runExternalScript(const QString& path);
    virtual QString scriptFileFilter();
    virtual QString highlightingMode();
};

class SageCASExtension : public Cantor::CASExtension
{
  public:
    SageCASExtension( QObject* parent);
    ~SageCASExtension();

  public Q_SLOTS:
    virtual QString solve(const QStringList& equations, const QStringList& variables);
    virtual QString simplify(const QString& expression);
    virtual QString expand(const QString& expression);

};

class SageCalculusExtension : public Cantor::CalculusExtension
{
  public:
    SageCalculusExtension( QObject* parent);
    ~SageCalculusExtension();

  public Q_SLOTS:
    QString limit(const QString& expression, const QString& variable, const QString& limit);
    QString differentiate(const QString& function,const QString& variable, int times);
    QString integrate(const QString& function, const QString& variable);
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right);
};

class SageLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    SageLinearAlgebraExtension( QObject* parent);
    ~SageLinearAlgebraExtension();

  public Q_SLOTS:
    //Commands to create Vectors/Matrices
    QString createVector(const QStringList& entries, VectorType type);
    QString nullVector(int size, VectorType type);
    QString createMatrix(const Matrix& matrix);
    QString identityMatrix(int size);
    QString nullMatrix(int rows,int columns);

    //basic functions
    QString rank(const QString& matrix);
    QString invertMatrix(const QString& matrix);
    QString charPoly(const QString& matrix);
    QString eigenVectors(const QString& matrix);
    QString eigenValues(const QString& matrix);
};

class SagePlotExtension : public Cantor::PlotExtension
{
  public:
    SagePlotExtension(QObject* parent);
    ~SagePlotExtension();
  public Q_SLOTS:
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right);
    QString plotFunction3d(const QString& function, VariableParameter var1, VariableParameter var2);
};

class SagePackagingExtension : public Cantor::PackagingExtension
{
  public:
    SagePackagingExtension(QObject* parent);
    ~SagePackagingExtension();

  public Q_SLOTS:
    virtual QString importPackage(const QString& module);
};

#endif /* _SAGEEXTENSIONS_H */
