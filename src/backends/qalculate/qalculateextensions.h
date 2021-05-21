/*
    SPDX-FileCopyrightText: 2011 Matteo Agostinelli <agostinelli@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QALCULATEEXTENSIONS_H
#define QALCULATEEXTENSIONS_H

#include <extension.h>

#define QALCULATE_EXT_CDTOR_DECL(name) Qalculate##name##Extension(QObject* parent); \
                                       ~Qalculate##name##Extension();

class QalculateHistoryExtension : public Cantor::HistoryExtension
{
public:
    QALCULATE_EXT_CDTOR_DECL(History)
    QString lastResult() override;
};

class QalculateVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    QALCULATE_EXT_CDTOR_DECL(VariableManagement)
    QString addVariable(const QString& name, const QString& value) override;
    QString setValue(const QString& name, const QString& value) override;
    QString removeVariable(const QString& name) override;
    QString saveVariables(const QString& fileName) override;
    QString loadVariables(const QString& fileName) override;
    QString clearVariables() override;
};

class QalculatePlotExtension : public Cantor::Extension
{
public:
    QALCULATE_EXT_CDTOR_DECL(Plot)
};

class QalculateCASExtension : public Cantor::CASExtension
{
  public:
    QALCULATE_EXT_CDTOR_DECL(CAS)

  public Q_SLOTS:
    QString solve(const QStringList& equations, const QStringList& variables) override;
    QString simplify(const QString& expression) override;
    QString expand(const QString& expression) override;

};

class QalculateCalculusExtension : public Cantor::CalculusExtension
{
  public:
    QALCULATE_EXT_CDTOR_DECL(Calculus)

  public Q_SLOTS:
    QString limit(const QString& expression, const QString& variable, const QString& limit) override;
    QString differentiate(const QString& function,const QString& variable, int times) override;
    QString integrate(const QString& function, const QString& variable) override;
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right) override;
};

class QalculateLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    QALCULATE_EXT_CDTOR_DECL(LinearAlgebra)

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

#endif /* QALCULATEEXTENSIONS_H */
