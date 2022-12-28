/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
        SPDX-FileCopyrightText: 2018-2022 by Alexander Semke (alexander.semke@web.de)
*/

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <QObject>
#include <QDomElement>

#include "cantor_export.h"

class QFileSystemWatcher;
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
 * Evaluation of Expression is an asynchronous process in most cases, so most of the members
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
    enum Status {
        Queued,      ///< The Expression is in expression queue, waited for Computing
        Computing,   ///< The Expression is still being computed
        Done,        ///< The Running of the Expression is finished successfully
        Error,       ///< An Error occurred when running the Expression
        Interrupted  ///< The Expression was interrupted by the user while running
    };

    /**
     * Enum indicating how this Expression behaves on finishing
     */
    enum FinishingBehavior {
        DoNotDelete,     ///< This Expression will not be deleted. This is the normal behaviour
        DeleteOnFinish   /** < The Object will delete itself when finished. This is used for fire-and-forget commands.
                          * All output/results will be dropped
                          */
    };
    /**
     * Expression constructor. Should only be called from Session::evaluateExpression
     * @param session the session, this Expression belongs to
     * @param internal \c true if this expression is internal expression
     */
    explicit Expression(Session*, bool internal = false);
    /**
     * destructor
     */
    ~Expression() override;

    /**
     * Evaluate the Expression. before this is called, you should set the Command first
     * This method can be implemented asynchronous, thus the Evaluation doesn't need to happen in the method,
     * It can also only be scheduled for evaluating.
     * @see setCommand()
     */
    virtual void evaluate() = 0;
    /**
     * Interrupt the running of the Expression.
     * This should set the state to Interrupted.
     */
    virtual void interrupt();

    virtual void parseOutput(const QString&) = 0;
    virtual void parseError(const QString&) = 0;

    /**
     * Returns the unique id of the Expression
     * or -1 for internal expressions
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
    void setFinishingBehavior(FinishingBehavior);

    /**
     * get the Expressions finishing behaviour
     * @return the current finishing behaviour
     */
    FinishingBehavior finishingBehavior();

    /**
     * Sets the command, represented by this Expression
     * @param cmd the command
     */
    void setCommand(const QString&);

    /**
     * Returns the command, represented by this Expression
     * @return the command, represented by this Expression
     */
    QString command();

    /**
     * Returns the command, adapted for using by appropriate Backend
     * The return value can be equal or not to @ref command()
     * Backend should use this function, instead of @ref command()
     */
    virtual QString internalCommand();

    /**
     * Adds some additional information/input to this expression.
     * this is needed, when the Expression has emitted the needsAdditionalInformation signal,
     * and the user has answered the question. This is used for e.g. if maxima asks whether
     * n+1 is zero or not when running the command "integrate(x^n,x)"
     * This method is part of the InteractiveMode feature
     */
    virtual void addInformation(const QString&);

    /**
     * Sets the error message
     * @param cmd the error message
     * @see errorMessage()
     */
    void setErrorMessage(const QString&);

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
    void removeResult(Result* result);

    /**
     * Deletes the all results of this expression.
     *
     */
    void clearResults();

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
    void setStatus(Status);

    /**
     * Returns the Session, this Expression belongs to
     */
    Session* session();

    /**
     * returns whether or not this expression is internal, or
     * comes from the user
     */
    bool isInternal() const;

    /**
     * Sets whether the expression is a help request (available for Maxima and R) where
     * additional information/help can be requested and shown.
     * Used internally to controll whether the update of the variable model needs to be done.
     */
    void setIsHelpRequest(bool);
    bool isHelpRequest() const;

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
     * emitted when the results of the expression were deleted.
     * @see clearResults()
     */
    void resultsCleared();
    /**
     * emitted when the results of the expression were deleted.
     * @see clearResult(Result* result)
     */
    void resultRemoved(int index);
    /**
     * emitted when the result at the position @c index was replaced by a new result.
     */
    void resultReplaced(int index);
    /**
     * the status of the Expression has changed.
     * @param status the new status
     */
    void statusChanged(Cantor::Expression::Status);

    /**
     * the status of the Expression has changed to Done, Error or Interrupt
     */
    void expressionFinished(Cantor::Expression::Status);

    /**
     * the Expression needs more information for the evaluation
     * @see addInformation()
     * @param question question, the user needs to answer
     */
    void needsAdditionalInformation(const QString& question);

  //These are protected, because only subclasses will handle results/status changes
  protected:
    // Protected constructor, useful for derived classes with own id setting strategy
    Expression(Session* session, bool internal, int id);

    /**
     * Set the result of the Expression.
     * this will cause gotResult() to be emitted
     * The old result will be deleted, and the Expression
     * takes over ownership of the result object, taking
     * care of deleting it.
     * @param result the new result
     */
    void setResult(Result*);
    void addResult(Result*);
    void replaceResult(int index, Result*);

    //returns a string of latex commands, that is inserted into the header.
    //used for example if special packages are needed
    virtual QString additionalLatexHeaders();

    QFileSystemWatcher* fileWatcher();

  private:
    void renderResultAsLatex(Result*);
    void latexRendered(LatexRenderer*, Result*);

  private:
    ExpressionPrivate* d;
};

}
#endif /* _EXPRESSION_H */
