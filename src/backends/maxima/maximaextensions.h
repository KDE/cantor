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
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMAEXTENSIONS_H
#define _MAXIMAEXTENSIONS_H

#include "extension.h"

class MaximaHistoryExtension : public Cantor::HistoryExtension
{
  public:
    MaximaHistoryExtension( QObject* parent );
    ~MaximaHistoryExtension() override;
  public Q_SLOTS:
    QString lastResult() Q_DECL_OVERRIDE;
};

class MaximaScriptExtension : public Cantor::ScriptExtension
{
  public:
    MaximaScriptExtension(QObject* parent);
    ~MaximaScriptExtension() override;
  public Q_SLOTS:
    QString runExternalScript(const QString& file) Q_DECL_OVERRIDE;
    QString scriptFileFilter() Q_DECL_OVERRIDE;
    QString highlightingMode() Q_DECL_OVERRIDE;
    QString commentStartingSequence() Q_DECL_OVERRIDE;
    QString commentEndingSequence() Q_DECL_OVERRIDE;
};


class MaximaCASExtension : public Cantor::CASExtension
{
  public:
    MaximaCASExtension( QObject* parent);
    ~MaximaCASExtension() override;

  public Q_SLOTS:
    QString solve(const QStringList& equations, const QStringList& variables) Q_DECL_OVERRIDE;
    QString simplify(const QString& expression) Q_DECL_OVERRIDE;
    QString expand(const QString& expression) Q_DECL_OVERRIDE;

};

class MaximaCalculusExtension : public Cantor::CalculusExtension
{
  public:
    MaximaCalculusExtension( QObject* parent);
    ~MaximaCalculusExtension() override;

  public Q_SLOTS:
    QString limit(const QString& expression, const QString& variable, const QString& limit) Q_DECL_OVERRIDE;
    QString differentiate(const QString& function,const QString& variable, int times) Q_DECL_OVERRIDE;
    QString integrate(const QString& function, const QString& variable) Q_DECL_OVERRIDE;
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right) Q_DECL_OVERRIDE;
};

/** An extension for basic Linear Algebra
 **/
class MaximaLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    MaximaLinearAlgebraExtension(QObject* parent);
    ~MaximaLinearAlgebraExtension() override;

  public Q_SLOTS:
    //Commands to create Vectors/Matrices
    QString createVector(const QStringList& entries, VectorType type) Q_DECL_OVERRIDE;
    QString createMatrix(const Matrix& matrix) Q_DECL_OVERRIDE;
    QString identityMatrix(int size) Q_DECL_OVERRIDE;

    //basic functions
    QString rank(const QString& matrix) Q_DECL_OVERRIDE;
    QString invertMatrix(const QString& matrix) Q_DECL_OVERRIDE;
    QString charPoly(const QString& matrix) Q_DECL_OVERRIDE;
    QString eigenVectors(const QString& matrix) Q_DECL_OVERRIDE;
    QString eigenValues(const QString& matrix) Q_DECL_OVERRIDE;

};

class MaximaPlotExtension : public Cantor::PlotExtension
{
  public:
    MaximaPlotExtension(QObject* parent);
    ~MaximaPlotExtension() override;
  public Q_SLOTS:
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) Q_DECL_OVERRIDE;
    QString plotFunction3d(const QString& function, VariableParameter var1, VariableParameter var2) Q_DECL_OVERRIDE;
};

class MaximaVariableManagementExtension : public Cantor::VariableManagementExtension
{
  public:
    MaximaVariableManagementExtension( QObject* parent );
    ~MaximaVariableManagementExtension() override;

  public Q_SLOTS:
    QString addVariable(const QString& name, const QString& value) Q_DECL_OVERRIDE;
    QString setValue(const QString& name,const QString& value) Q_DECL_OVERRIDE;
    QString removeVariable(const QString& name) Q_DECL_OVERRIDE;

    QString saveVariables(const QString& fileName) Q_DECL_OVERRIDE;
    QString loadVariables(const QString& fileName) Q_DECL_OVERRIDE;
    QString clearVariables() Q_DECL_OVERRIDE;
};


#endif /* _MAXIMAEXTENSIONS_H */
