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
    ~MaximaHistoryExtension();
  public slots:
    QString lastResult();
};

class MaximaScriptExtension : public Cantor::ScriptExtension
{
  public:
    MaximaScriptExtension(QObject* parent);
    ~MaximaScriptExtension();
  public slots:
    virtual QString runExternalScript(const QString& file);
    virtual QString scriptFileFilter();
    virtual QString highlightingMode();
    QString commentStartingSequence();
    QString commentEndingSequence();
};


class MaximaCASExtension : public Cantor::CASExtension
{
  public:
    MaximaCASExtension( QObject* parent);
    ~MaximaCASExtension();

  public slots:
    virtual QString solve(const QStringList& equations, const QStringList& variables);
    virtual QString simplify(const QString& expression);
    virtual QString expand(const QString& expression);

};

class MaximaCalculusExtension : public Cantor::CalculusExtension
{
  public:
    MaximaCalculusExtension( QObject* parent);
    ~MaximaCalculusExtension();

  public slots:
    QString limit(const QString& expression, const QString& variable, const QString& limit);
    QString differentiate(const QString& function,const QString& variable, int times);
    QString integrate(const QString& function, const QString& variable);
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right);
};

/** An extension for basic Linear Algebra
 **/
class MaximaLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    MaximaLinearAlgebraExtension(QObject* parent);
    ~MaximaLinearAlgebraExtension();

  public slots:
    //Commands to create Vectors/Matrices
    virtual QString createVector(const QStringList& entries, VectorType type);
    virtual QString createMatrix(const Matrix& matrix);
    virtual QString identityMatrix(int size);

    //basic functions
    virtual QString rank(const QString& matrix);
    virtual QString invertMatrix(const QString& matrix);
    virtual QString charPoly(const QString& matrix);
    virtual QString eigenVectors(const QString& matrix);
    virtual QString eigenValues(const QString& matrix);

};

class MaximaPlotExtension : public Cantor::PlotExtension
{
  public:
    MaximaPlotExtension(QObject* parent);
    ~MaximaPlotExtension();
  public slots:
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right);
    QString plotFunction3d(const QString& function, VariableParameter var1, VariableParameter var2);
};

class MaximaVariableManagementExtension : public Cantor::VariableManagementExtension
{
  public:
    MaximaVariableManagementExtension( QObject* parent );
    ~MaximaVariableManagementExtension();

  public slots:
    virtual QString addVariable(const QString& name, const QString& value);
    virtual QString setValue(const QString& name,const QString& value);
    virtual QString removeVariable(const QString& name);

    virtual QString saveVariables(const QString& fileName);
    virtual QString loadVariables(const QString& fileName);
    virtual QString clearVariables();
};


#endif /* _MAXIMAEXTENSIONS_H */
