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
#include <QPair>
#include "cantor_export.h"

namespace Cantor
{
/**
 * This is the base class for all Extensions.
 * An Extension provides a set of Methods to
 * accomplish specific tasks. This is used to
 * abstract away the backends syntax for common
 * tasks like solving equations etc. to be able
 * to provide Backend independent Dialogs
 *
 * @author Alexander Rieder
 */
class CANTOR_EXPORT Extension : public QObject
{
  public:
    ///Default constructor
    Extension( const QString& name, QObject* parent );
    ~Extension();

};

//Some basic interfaces for extensions

/**
 * An Extension providing commands for command history
 */
class CANTOR_EXPORT HistoryExtension : public Extension
{
  public:
    HistoryExtension(QObject* parent);
    ~HistoryExtension();
  public slots:
    /**
     * Returns a command that retrieves the last result
     * @return command that retrieves the last result
     */
    virtual QString lastResult() = 0;
};

/**
 * An Extension providing commands to interact
 * with external scripts
 */
class CANTOR_EXPORT ScriptExtension : public Extension
{
  public:
    ScriptExtension(QObject* parent);
    ~ScriptExtension();
  public slots:
    /**
     * returns the command for running a script
     * @param path path to the script file
     * @return command for running a script
     */
    virtual QString runExternalScript(const QString& path) = 0;
    /**
     * returns the file filter used for Script Files (e.g. *.py)
     * @return file filter used for Script Files (e.g. *.py)
     */
    virtual QString scriptFileFilter() = 0;

    /**
     * returns a string used to separate commands (usually ;)
     * @return a string used to separate commands (usually ;)
     */
    virtual QString commandSeparator();
    /**
     * returns a string used to start a comment (usually #)
     * @return a string used to start a comment (usually #)
     */
    virtual QString commentStartingSequence();
    /**
     * returns a string used to end a comment (usually "")
     * @return a string used to end a comment (usually "")
     */
    virtual QString commentEndingSequence();
};

/**
 * An extension providing the basic computations
 * in computer algebra, like solving, simplifying
 * etc
**/
class CANTOR_EXPORT CASExtension : public Extension
{
  public:
    CASExtension(QObject* parent);
    ~CASExtension();

  public slots:
    /**
     * returns the command for solving a set of equations
     * @param equations a list of equations
     * @param variables a list of variables that should be solved for
     * @return command for solving a set of equations
     */
    virtual QString solve(const QStringList& equations, const QStringList& variables) = 0;
    /**
     * returns the command for simplifying an expression
     * @param expression the expression that should be simplified
     * @return command for simplifying the expression
     */
    virtual QString simplify(const QString& expression) = 0;
    /**
     * returns the command for expanding an expression
     * @param expression the expression that should be expanded
     * @return command for expanded the expression
     */
    virtual QString expand(const QString& expression) = 0;
};

/**
 * An extension providing the basic calculus
 * stuff like limits, diffrentiate, integrate etc.
 */
class CANTOR_EXPORT CalculusExtension : public Extension
{
  public:
    CalculusExtension(QObject* parent);
    ~CalculusExtension();

  public slots:
    /**
     * returns the command for calculating a limit if an expression
     * @param expression the expression
     * @param variable the variable
     * @param limit the value, the variable approaches
     * @return the limit of the expression
     */
    virtual QString limit(const QString& expression, const QString& variable, const QString& limit) = 0;
    /**
     * returns the command for calculating a differential
     * @param function the function
     * @param variable the variable, after which shoudl be differentiated
     * @param times how often should be differentiated
     * @return the command to compute the differential
     */
    virtual QString differentiate(const QString& function,const QString& variable, int times) = 0;
    /**
     * returns the command for calculating an integral
     * @param function the function
     * @param variable the variable, after which shoudl be integrated
     * @return the command to compute the integrate
     */
    virtual QString integrate(const QString& function, const QString& variable) = 0;
    /**
     * returns the command for calculating a definite integral
     * @param function the function
     * @param variable the variable, after which shoudl be integrated
     * @param left the left border of the integral
     * @param right the right border of the integral
     * @return the command to compute the integrate
     */
    virtual QString integrate(const QString& function,const QString& variable, const QString& left, const QString& right) = 0;
};

/**
 * An extension providing basic plotting facilities
 */
class CANTOR_EXPORT PlotExtension : public Extension
{
  public:
    typedef QPair<QString,QString> Interval;
    typedef QPair<QString, Interval> VariableParameter;

    PlotExtension(QObject* parent);
    ~PlotExtension();

  public slots:
    /**
     * returns the command for plotting a 2 dimensional function.
     * @param function the function to plot
     * @param variable the variable
     * @param left the left border of the plot
     * @param right the right border of the plot
     * @return the command for plotting
     */
    virtual QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) = 0;
    /**
     * returns the command for plotting a 3 dimensional function.
     * @param function the function to plot
     * @param var1 the parameters for Variable1 (name, interval)
     * @param var2 the parameters for Variable2 (name, interval)
     * @return the command for plotting
     */
    virtual QString plotFunction3d(const QString& function, VariableParameter var1, VariableParameter var2) = 0;
};

/**
 * An extension for basic Linear Algebra
 */
class CANTOR_EXPORT LinearAlgebraExtension : public Extension
{
  public:
    enum VectorType { ColumnVector, RowVector };
    typedef QList<QStringList> Matrix;

    LinearAlgebraExtension(QObject* parent);
    ~LinearAlgebraExtension();

  public slots:
    //Commands to create Vectors/Matrices
    /**
     * creates a vector with the given entries
     * @param entries the entries of the new vector
     * @param type the type of the vector (row/column)
     * @return the command for creating the vector
     */
    virtual QString createVector(const QStringList& entries, VectorType type) = 0;
    /**
     * creates a null vector, of the given size/type
     * @param size size of the vector
     * @param type type of the vector
     * @return the command used for creating a nullvector
     **/
    virtual QString nullVector(int size, VectorType type);
    /**
     * creates a maxtrix with the given entries
     * @param matrix the entries of the matrix
     * @return the command to create this matrix
     */
    virtual QString createMatrix(const Matrix& matrix) = 0;
    /**
     * creates an identity matrix of the given size
     * @param size size of the matrix
     * @return the command used to create the matrix
     */
    virtual QString identityMatrix(int size);
    /**
     * creates a null matrix, of the given size
     * @param rows number of rows
     * @param columns number of columns
     * @return the command to create this matrix
     */
    virtual QString nullMatrix(int rows,int columns);

    //basic functions
    /**
     * compute the rank of a matrix
     * @param matrix the name of the matrix, the rank should be computed of
     * @return the command for calculating the rank
     */
    virtual QString rank(const QString& matrix) = 0;
    /**
     * invert a given matrix
     * @param matrix the name of the matrix, that should be inverted
     * @return the command for inverting the matrix
     */
    virtual QString invertMatrix(const QString& matrix) = 0;
    /**
     * calculate the characteristic polynom of a matrix
     * @param matrix the name of the matrix, the charpoly should be computed of
     * @return the command
     */
    virtual QString charPoly(const QString& matrix) = 0;
    /**
     * calculate the eigen vectors of a matrix
     * @param matrix the name of the matrix, the eigenvectors should be computed of
     * @return the command
     */
    virtual QString eigenVectors(const QString& matrix) = 0;
    /**
     * calculate the eigen values of a matrix
     * @param matrix the name of the matrix, the eigenvalues should be computed of
     * @return the command
     */
    virtual QString eigenValues(const QString& matrix) = 0;

};

}
#endif /* _EXTENSION_H */
