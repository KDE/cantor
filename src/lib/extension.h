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

#ifndef _EXTENSION_H
#define _EXTENSION_H

#include <QObject>
#include "mathematik_export.h"

namespace MathematiK
{
/** This is the base class for all Extensions.
    An Extension provides a set of Methods to
    accomplish specific tasks. This is used to
    abstract away the backends syntax for common
    tasks like solving equations etc. to be able
    to provide Backend independent Dialogs
**/
class MATHEMATIK_EXPORT Extension : public QObject
{
  public:
    Extension( const QString& name, QObject* parent );
    ~Extension();

};

//Some basic interfaces for extensions

/** An Extension providing commands for command history
 **/
class MATHEMATIK_EXPORT HistoryExtension : public Extension
{
  public:
    HistoryExtension(QObject* parent);
    ~HistoryExtension();
  public slots:
    //Returns a command that retrieves the last result
    virtual QString lastResult() = 0;
};

/** An extension providing the basic computations
    in computer algebra, like solving, simplifying
    etc
**/
class MATHEMATIK_EXPORT CASExtension : public Extension
{
  public:
    CASExtension(QObject* parent);
    ~CASExtension();

  public slots:
    virtual QString solve(const QStringList& equations, const QStringList& variables) = 0;
    virtual QString simplify(const QString& expression) = 0;
    virtual QString expand(const QString& expression) = 0;
};

/** An extension providing the basic calculus
    stuff like limits, diffrentiate, integrate etc.
**/
class MATHEMATIK_EXPORT CalculusExtension : public Extension
{
  public:
    CalculusExtension(QObject* parent);
    ~CalculusExtension();

  public slots:
    virtual QString limit(const QString& expression, const QString& variable, const QString& limit) = 0;
    virtual QString differentiate(const QString& function,const QString& variable, int times) = 0;
    virtual QString integrate(const QString& function, const QString& variable) = 0;
    virtual QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right) = 0; 
};

/** An extension providing basic plotting facilities
 **/
class MATHEMATIK_EXPORT PlotExtension : public Extension
{
  public:
    PlotExtension(QObject* parent);
    ~PlotExtension();
  
  public slots:
    virtual QString plotFunction2D(const QString& function, const QString& variable, const QString& left, const QString& right) = 0;
};

/** An extension for basic Linear Algebra
 **/
class MATHEMATIK_EXPORT LinearAlgebraExtension : public Extension
{
  public:
    enum VectorType { ColumnVector, RowVector };
    typedef QList<QStringList> Matrix;

    LinearAlgebraExtension(QObject* parent);
    ~LinearAlgebraExtension();

  public slots:
    //Commands to create Vectors/Matrices
    virtual QString createVector(const QStringList& entries, VectorType type) = 0;
    virtual QString nullVector(int size, VectorType type);
    virtual QString createMatrix(const Matrix& matrix) = 0;
    virtual QString identityMatrix(int size);
    virtual QString nullMatrix(int rows,int columns);

    //basic functions
    virtual QString rank(const QString& matrix) = 0;
    virtual QString invertMatrix(const QString& matrix) = 0;
    virtual QString charPoly(const QString& matrix) = 0;
    virtual QString eigenVectors(const QString& matrix) = 0;
    virtual QString eigenValues(const QString& matrix) = 0;

};

}
#endif /* _EXTENSION_H */
