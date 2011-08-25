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

#ifndef _QALCULATEEXTENSIONS_H
#define _QALCULATEEXTENSIONS_H

#include "extension.h"

class QalculateCASExtension : public Cantor::CASExtension
{
  public:
    QalculateCASExtension(QObject* parent);
    ~QalculateCASExtension();

  public slots:
    virtual QString solve(const QStringList& equations, const QStringList& variables);
    virtual QString simplify(const QString& expression);
    virtual QString expand(const QString& expression);

};

class QalculateCalculusExtension : public Cantor::CalculusExtension
{
  public:
    QalculateCalculusExtension(QObject* parent);
    ~QalculateCalculusExtension();

  public slots:
    QString limit(const QString& expression, const QString& variable, const QString& limit);
    QString differentiate(const QString& function,const QString& variable, int times);
    QString integrate(const QString& function, const QString& variable);
    QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right);
};

class QalculateLinearAlgebraExtension : public Cantor::LinearAlgebraExtension
{
  public:
    QalculateLinearAlgebraExtension(QObject* parent);
    ~QalculateLinearAlgebraExtension();

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


#endif /* _QALCULATEEXTENSIONS_H */
