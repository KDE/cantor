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
#include <QVector>
#include <QDebug>
#include <QWidget>
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
  Q_OBJECT
  public:
    ///Default constructor
    Extension( const QString& name, QObject* parent );
    ~Extension() override = default;
};


//Some basic interfaces for extensions

/**
 * An Extension providing commands for command history
 */
class CANTOR_EXPORT HistoryExtension : public Extension
{
  Q_OBJECT
  public:
    HistoryExtension(QObject* parent);
    ~HistoryExtension() override;
  public Q_SLOTS:
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
  Q_OBJECT
  public:
    ScriptExtension(QObject* parent);
    ~ScriptExtension() override;
  public Q_SLOTS:
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
     * returns the name of the language to use for syntax highlighting
     * in the script editor (e.g. python). The value returned must match
     * the name attribute from the xml language description in KTexteditor.
     * @return name of the language to use for syntax highlighting (e.g. python)
     */
    virtual QString highlightingMode() = 0;
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
  Q_OBJECT
  public:
    CASExtension(QObject* parent);
    ~CASExtension() override;

  public Q_SLOTS:
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
 * stuff like limits, differentiate, integrate etc.
 */
class CANTOR_EXPORT CalculusExtension : public Extension
{
  Q_OBJECT
  public:
    CalculusExtension(QObject* parent);
    ~CalculusExtension() override;

  public Q_SLOTS:
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
     * @param variable the variable, after which should be differentiated
     * @param times how often should be differentiated
     * @return the command to compute the differential
     */
    virtual QString differentiate(const QString& function,const QString& variable, int times) = 0;
    /**
     * returns the command for calculating an integral
     * @param function the function
     * @param variable the variable, after which should be integrated
     * @return the command to compute the integrate
     */
    virtual QString integrate(const QString& function, const QString& variable) = 0;
    /**
     * returns the command for calculating a definite integral
     * @param function the function
     * @param variable the variable, after which should be integrated
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
  Q_OBJECT
  public:
    typedef QPair<QString,QString> Interval;
    typedef QPair<QString, Interval> VariableParameter;

    PlotExtension(QObject* parent);
    ~PlotExtension() override;

  public Q_SLOTS:
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
    virtual QString plotFunction3d(const QString& function, const VariableParameter& var1, const VariableParameter& var2) = 0;
};

#define PLOT_DIRECTIVE_DISPATCHING(x) QString dispatch(const Cantor::AdvancedPlotExtension::AcceptorBase& acc) const \
  { \
    const Cantor::AdvancedPlotExtension::DirectiveAcceptor<x>* adaptor= \
        dynamic_cast<const Cantor::AdvancedPlotExtension::DirectiveAcceptor<x>*>(&acc); \
    if (adaptor==NULL) { qDebug()<<"Backend incapable of processing directives of type "#x;  return QLatin1String(""); } \
    else \
        return adaptor->accept(*this); \
 }

/**
 * An extension providing advanced plotting facilities. Will supersede PlotExtension
 */
class CANTOR_EXPORT AdvancedPlotExtension : public Extension
{
  Q_OBJECT
  public:
    AdvancedPlotExtension(QObject* parent);
    ~AdvancedPlotExtension() override;

    // TODO comment

    class PlotDirective;

    // TODO move the hell out of here
    class CANTOR_EXPORT DirectiveProducer : public QWidget
    {
        public:
            DirectiveProducer(QWidget* parent);
            virtual PlotDirective* produceDirective() const=0;
    };

    template <class UI> class DirectiveControl : protected UI, public DirectiveProducer
    {
        public:
            DirectiveControl(QWidget* parent) : DirectiveProducer(parent) { UI::setupUi(this); }
        protected:
            using AbstractParent = DirectiveControl<UI>;
    };

    class CANTOR_EXPORT AcceptorBase
    {
        public:
            /**
             * utilitary typename for easing the code
             */
            using widgetProc = DirectiveProducer *(*)(QWidget *);

            /**
             * returns a constant reference to the list of widget generating procedures
             * which contains means of creating all the widgets a backend knows how to process
             * @return the constant reference to a QVector of QWidget* (*)(QWidget*) pointers
             */
            const QVector<widgetProc>& widgets() const;

        protected:
            /**
             * constructor only allowed for derived classes
             **/
            AcceptorBase();
            virtual ~AcceptorBase() = default;

            QVector<widgetProc> m_widgets;
    };

    template <class Directive> class DirectiveAcceptor : virtual public AcceptorBase
    {

        public:
            /**
             * virtual interface to acceptor function mechanics
             * @param directive the directive to process
             * @return the parameter corresponding the directive
             */
            virtual QString accept(const Directive& directive) const=0;

        protected:
            /**
             * constructor only allowed for derived classes
             **/
            DirectiveAcceptor();
    };

    class CANTOR_EXPORT PlotDirective
    {
        public:
            virtual ~PlotDirective() = default;

            /**
             * creates a new widget for editing the value and returns the pointer to it
             * @param parent the pointer to parent widget passed to newly created widget
             * @return pointer to the newly-created widget
             */
            static QWidget* widget(QWidget* parent);

            /**
             * in order to make dual dispatching this should be present in any derived class
             * without virtual keyword and with correct class name
             **/
            virtual PLOT_DIRECTIVE_DISPATCHING(PlotDirective);
            // TODO: find a workaround not to put class names manually

        protected:
            /**
             * only derived classes may construct
             **/
            PlotDirective() = default;
    };

  public Q_SLOTS:
    /**
     * returns the command for plotting a 2 dimensional data set.
     * @param expression the expression to plot
     * @param directives the array of directives toward the generator
     * @return the command for plotting
     */
    QString plotFunction2d(const QString& expression, const QVector<Cantor::AdvancedPlotExtension::PlotDirective*>& directives) const;

    /**
     * returns the parameter expression according to a directive.
     * @param directive the directive toward the generator
     * @return the parameter for plotting
     */
    QString dispatchDirective(const Cantor::AdvancedPlotExtension::PlotDirective& directive) const;

  protected:
    /**
     * returns the command name for plotting a 2 dimensional data set.
     * @return the command for plotting
     */
    virtual QString plotCommand() const=0;
    /**
     * returns the separator symbol in a plotting command.
     * @return the separator symbol or string
     */
    virtual QString separatorSymbol() const;
};

template <class Directive> AdvancedPlotExtension::DirectiveAcceptor<Directive>::DirectiveAcceptor()
{
    m_widgets.push_back(&Directive::widget);
}

/**
 * An extension for basic Linear Algebra
 */
class CANTOR_EXPORT LinearAlgebraExtension : public Extension
{
  Q_OBJECT
  public:
    enum VectorType { ColumnVector, RowVector };
    using Matrix = QList<QStringList>;

    LinearAlgebraExtension(QObject* parent);
    ~LinearAlgebraExtension() override;

  public Q_SLOTS:
    //Commands to create Vectors/Matrices
    /**
     * creates a vector with the given entries
     * @param entries the entries of the new vector
     * @param type the type of the vector (row/column)
     * @return the command for creating the vector
     */
    virtual QString createVector(const QStringList& entries, Cantor::LinearAlgebraExtension::VectorType type) = 0;
    /**
     * creates a null vector, of the given size/type
     * @param size size of the vector
     * @param type type of the vector
     * @return the command used for creating a nullvector
     **/
    virtual QString nullVector(int size, Cantor::LinearAlgebraExtension::VectorType type);
    /**
     * creates a matrix with the given entries
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

class CANTOR_EXPORT VariableManagementExtension : public Extension
{
  Q_OBJECT
  public:
    VariableManagementExtension( QObject* parent );
    ~VariableManagementExtension() override;

  public Q_SLOTS:
    virtual QString addVariable(const QString& name, const QString& value) = 0;
    virtual QString setValue(const QString& name,const QString& value) = 0;
    virtual QString removeVariable(const QString& name) = 0;

    virtual QString saveVariables(const QString& fileName) = 0;
    virtual QString loadVariables(const QString& fileName) = 0;
    virtual QString clearVariables() = 0;
};

/**
 * An extension for library/module import
 */
class CANTOR_EXPORT PackagingExtension : public Extension
{
  Q_OBJECT
  public:
    PackagingExtension(QObject* parent);
    ~PackagingExtension() override;

  public Q_SLOTS:
    /**
     * import library/module
     * @param package the library/module name
     * @return the command for import library/module
     */
    virtual QString importPackage(const QString& package) = 0;
};

}
#endif /* _EXTENSION_H */
