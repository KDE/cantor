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

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <QObject>
#include <QDomElement>

#include "cantor_export.h"

class KZip;

/**
 * Namespace collecting all Classes of the Cantor Libraries
 */
namespace Cantor
{
class Session;
class Result;
class ExpressionPrivate;

/**
 * An Expression object is used, to store the information needed when running a command of a Session
 * Evaluation of Expression is an asynchroneous process in most cases, so most of the members
 * of this class are not useful directly after its construction. Therefore there are signals
 * indicating, when the Expression goes through the different stages of the Running process.
 * An Expression is never constructed directly, but by using Session::evaluateExpression()
 *
 * @author Alexander Rieder
 */
class CANTOR_EXPORT Expression : public QObject
{
  Q_OBJECT
  public:
    enum Status{ Computing,   ///< The Expression is still being computed
                 Done,        ///< The Running of the Expression is finished sucessfully
                 Error,       ///< An Error occurred when running the Expression
                 Interrupted  ///< The Expression was interrupted by the user while running
    };

    /**
     * Enum indicating how this Expression behaves on finishing
     */
    enum FinishingBehavior {
        DoNotDelete,     ///< This Expression will not be deleted. This is the normal behaviour
        DeleteOnFinish   /** < The Object will delete itself when finished. This is used for fire-and-forget commands.
                               All output/results will be dropped
                         */
    };
    /**
     * Expression constructor. Should only be called from Session::evaulateExpression
     * @param session the session, this Expression belongs to
     */
    Expression( Session* session );
    /**
     * destructor
     */
    virtual ~Expression();

    /**
     * Evaluate the Expression. before this is called, you should set the Command first
     * This method can be implemented asynchroneous, thus the Evaluation doesn't need to happen in the method,
     * It can also only be scheduled for evaluating.
     * @see setCommand()
     */
    virtual void evaluate() = 0;
    /**
     * Interrupt the running of the Expression.
     * This should set the state to Interrupted.
     */
    virtual void interrupt() = 0;

    /**
     * Returns the unique id of the Expression
     * @return the unique id of the Expression
     */
    int id();

    /**
     * set the id of the Expression. It should be unique
     * @param id the new Id
     */
    void setId(int id);

    /**
     * set the finishing behaviour
     * @param behavior the new Finishing Behaviour
     */
    void setFinishingBehavior(FinishingBehavior behavior);

    /**
     * get the Expressions finishing behaviour
     * @return the current finishing behaviour
     */
    FinishingBehavior finishingBehavior();

    /**
     * Sets the command, represented by this Expression
     * @param cmd the command
     */
    void setCommand( const QString& cmd );

    /**
     * Returns the command, represented by this Expression
     * @return the command, represented by this Expression
     */
    QString command();

    /**
     * Adds some additional information/input to this expression.
     * this is needed, when the Expression has emitted the needsAdditionalInformation signal,
     * and the user has answered the question. This is used for e.g. if maxima asks wether
     * n+1 is zero or not when running the command "integrate(x^n,x)"
     * This method is part of the InteractiveMode feature
     */
    virtual void addInformation(const QString& information);

    /**
     * Sets the error message
     * @param cmd the error message
     * @see errorMessage()
     */
    void setErrorMessage( const QString& cmd);

    /**
     * returns the Error message, if an error occurred during
     * the evaluation of the expression.
     * @return the error message
     */
    QString errorMessage();

    /**
     * The results of this Expression. They can have different types, represented by various
     * subclasses of Result, like text, image, etc.
     * The list will be empty, until the computation is completed.
     * When the results change, the gotResult() signal is emitted.
     * The Result objects are owned by the Expression, and will get deleted, as
     * soon as the Expression dies, or newer results appear.
     * @return the results of the Expression
     */
    QList<Result*> results();

    /**
     * @return true if the Expression has results, false otherwise
     */
    bool hasResults();

    /**
     * Deletes the result of this expression.
     *
     */
    void clearResults();

    /**
     * Returns the status of this Expression
     * @return the status of this Expression
     */
    Status status();

    /**
     * Returns the Session, this Expression belongs to
     */
    Session* session();

    /**
     * returns an xml representation of this expression
     * used for saving the worksheet
     * @param doc DomDocument used for storing the information
     * @return QDomElemt containing the representation of this Expression
     */
    QDomElement toXml(QDomDocument& doc);
    /**
     * saves all the data, that can't be saved in xml
     * in an extra file in the archive. for Example
     * images of plots
     * @param archive a Zip archive, the data should be stored in
     */
    void saveAdditionalData(KZip* archive);

    /**
     * returns whether or not this expression is internal, or
     * comes from the user
     */
    bool isInternal();
    /**
     * mark this expression as an internal expression,
     * so for example latex will not be run on it
     */
    void setInternal(bool internal);

  Q_SIGNALS:
    /**
     * the Id of this Expression changed
     */
    void idChanged();
    /**
     * A Result of the Expression has arrived
     */
    void gotResult();
    /**
     * the status of the Expression has changed.
     * @param status the new status
     */
    void statusChanged(Cantor::Expression::Status status);
    /**
     * the Expression needs more information for the evaluation
     * @see addInformation()
     * @param question question, the user needs to answer
     */
    void needsAdditionalInformation(const QString& question);

  //These are protected, because only subclasses will handle results/status changes
  protected:
    /**
     * Set the result of the Expression.
     * this will cause gotResult() to be emited
     * The old results will be deleted, and the Expression
     * takes over ownership of the result object, taking
     * care of deleting it.
     * @param result the new result
     */
    void setResult(Result* result);
    /**
     * Set the results of the Expression.
     * This will cause gotResult() to be emited.
     * The old results will be deleted, and the Expression
     * takes over ownership of the new result objects.
     * @param results the new results
     */
    void setResults(QList<Result*> results);

    /**
     * Sets the result at index @index to @result
     * This will cause gotResult() to be emited.
     * The old result will be deleted, and the Expression
     * takes over ownership of the new result object.
     * @param results the new results
     */
    void setResult(Result* result,int index);
    /**
     * Set the status
     * statusChanged will be emitted
     * @param status the new status
     */
    void setStatus(Status status);

  protected:
    //returns a string of latex commands, that is inserted into the header.
    //used for example if special packages are needed
    virtual QString additionalLatexHeaders();
  private:
    void renderResultAsLatex();
  private Q_SLOTS:
    void latexRendered();

  private:
    ExpressionPrivate* d;
};

}
#endif /* _EXPRESSION_H */
