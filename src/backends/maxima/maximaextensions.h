/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _MAXIMAEXTENSIONS_H
#define _MAXIMAEXTENSIONS_H

#include "extension.h"

class MaximaHistoryExtension : public Cantor::HistoryExtension
{
  public:
    explicit MaximaHistoryExtension( QObject* parent );
    ~MaximaHistoryExtension() override;
  public Q_SLOTS:
    QString lastResult() override;
};

class MaximaScriptExtension : public Cantor::ScriptExtension
{
  public:
    explicit MaximaScriptExtension(QObject* parent);
    ~MaximaScriptExtension() override;
  public Q_SLOTS:
    QString runExternalScript(const QString& file) override;
    QString scriptFileFilter() override;
    QString highlightingMode() override;
    QString commentStartingSequence() override;
    QString commentEndingSequence() override;
};


class MaximaCASExtension : public Cantor::CASExtension
{
  public:
    explicit MaximaCASExtension( QObject* parent);
    ~MaximaCASExtension() override;

  public Q_SLOTS:
    QString solve(const QStringList& equations, const QStringList& variables) override;
    QString simplify(const QString& expression) override;
    QString expand(const QString& expression) override;

};

class MaximaCalculusExtension : public Cantor::CalculusExtension
{
  public:
    explicit MaximaCalculusExtension( QObject* parent);
    ~MaximaCalculusExtension() override;

  public Q_SLOTS:
    QString limit(const QString& expression, const QString& variable, const QString& limit) override;
    QString differentiate(const QString& function,const QString& variable, int times) override;
    QString integrate(const QString& function, const QString& variable) override;
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right) override;
};

/** An extension for basic Linear Algebra
 **/
class MaximaLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    explicit MaximaLinearAlgebraExtension(QObject* parent);
    ~MaximaLinearAlgebraExtension() override;

  public Q_SLOTS:
    //Commands to create Vectors/Matrices
    QString createVector(const QStringList& entries, VectorType type) override;
    QString createMatrix(const Matrix& matrix) override;
    QString identityMatrix(int size) override;

    //basic functions
    QString rank(const QString& matrix) override;
    QString invertMatrix(const QString& matrix) override;
    QString charPoly(const QString& matrix) override;
    QString eigenVectors(const QString& matrix) override;
    QString eigenValues(const QString& matrix) override;

};

class MaximaPlotExtension : public Cantor::PlotExtension
{
  public:
    explicit MaximaPlotExtension(QObject* parent);
    ~MaximaPlotExtension() override;
  public Q_SLOTS:
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) override;
    QString plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2) override;
};

class MaximaVariableManagementExtension : public Cantor::VariableManagementExtension
{
  public:
    explicit MaximaVariableManagementExtension( QObject* parent );
    ~MaximaVariableManagementExtension() override;

  public Q_SLOTS:
    QString addVariable(const QString& name, const QString& value) override;
    QString setValue(const QString& name,const QString& value) override;
    QString removeVariable(const QString& name) override;

    QString saveVariables(const QString& fileName) override;
    QString loadVariables(const QString& fileName) override;
    QString clearVariables() override;
};


#endif /* _MAXIMAEXTENSIONS_H */
