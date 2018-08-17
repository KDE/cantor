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
class LatexRenderer;
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
		 Done,        ///< The Running of the Expression is finished successfully
		 Error,       ///< An Error occurred when running the Expression
		 Interrupted,  ///< The Expression was interrupted by the user while running
         Queued
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
    ~Expression() override;

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
     * and the user has answered the question. This is used for e.g. if maxima asks whether
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
     * The result of this Expression. It can have different types, represented by various
     * subclasses of Result, like text, image, etc.
     * The result will be null, until the computation is completed.
     * When the result changes, the gotResult() signal is emitted.
     * The Result object is owned by the Expression, and will get deleted, as
     * soon as the Expression dies, or newer results appear.
     * @return the result of the Expression, 0 if it isn't yet set
     */
    Result* result();

    /*!
     * in case the expression has multiple outputs/results, those can be obtained with this functions.
     * Everything else said for \sa result() applies here too.
     * @return the vector with results, or an empty vector if nor results are available yet.
     */
    const QVector<Result*>& results() const;

    /**
     * Deletes the result of this expression.
     *
     */
    void clearResult();

    /**
     * Returns the status of this Expression
     * @return the status of this Expression
     */
    Status status();

    /**
     * Set the status
     * statusChanged will be emitted
     * @param status the new status
     */
    void setStatus(Status status);

    /**
     * Returns the Session, this Expression belongs to
     */
    Session* session();

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
     * this will cause gotResult() to be emitted
     * The old result will be deleted, and the Expression
     * takes over ownership of the result object, taking
     * care of deleting it.
     * @param result the new result
     */
    void setResult(Result* result);

    void addResult(Result*);

  protected:
    //returns a string of latex commands, that is inserted into the header.
    //used for example if special packages are needed
    virtual QString additionalLatexHeaders();
  private:
    void renderResultAsLatex(Result* result);
    void latexRendered(LatexRenderer* renderer, Result* result);

  private:
    ExpressionPrivate* d;
};

}
#endif /* _EXPRESSION_H */
