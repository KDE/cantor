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

#ifndef _SESSION_H
#define _SESSION_H

#include <QObject>

#include "cantor_export.h"

#include "expression.h"

class QTextEdit;
class QSyntaxHighlighter;
class QAbstractItemModel;

/**
 * Namespace collecting all Classes of the Cantor Libraries
 */
namespace Cantor
{
class Backend;
class SessionPrivate;
class CompletionObject;
class SyntaxHelpObject;

/**
 * The Session object is the main class used to interact with a Backend.
 * It is used to evaluate Expressions, get completions, syntax highlighting, etc.
 * 
 * @author Alexander Rieder
 */
class CANTOR_EXPORT Session : public QObject
{
  Q_OBJECT
  public:
    enum Status { Running, ///< the session is busy, running some expression
		  Done     ///< the session has done all the jobs, and is now waiting for more
    };

    /**
     * Create a new Session. This should not yet set up the complete session,
     * thats job of the login() function
     * @see login()
     */
    Session( Backend* backend);
    /**
     * Destructor
     */
    ~Session();

    /**
     * Login to the Session. In this function you should do anything needed to set up
     * the session, and make it ready for usage. The method should be implemented non
     * blocking. If the loging in is completed, the ready() signal must be emitted
     */
    virtual void login() = 0;
    /**
     * Log out of the Session. Destroy everything specific to a single session, e.g.
     * stop all the running processes etc.
     * NOTE: resatring the session consists of first logout() and then login()
     */
    virtual void logout() = 0;
    
    /**
     * Passes the given command to the backend and returns a Pointer 
     * to a new Expression object, which will emit the resultArrived() 
     * signal as soon as the computation is done. The result will 
     * then be acessible by Expression::result()
     * @param command the command that should be run by the backend.
     * @param finishingBehavior the FinishingBehaviour that should be used for this command. @see Expression::FinishingBehaviour
     * @return an Expression object, representing this command
     */
    virtual Expression* evaluateExpression(const QString& command, Expression::FinishingBehavior finishingBehavior) = 0;

    /**
     * Reimplements evaluateExpression, setting the finishingBehaviour to DoNotDelete
     * @see evaluateExpressionconst(QString& command, Expression::FinishingBehavior finishingBehavior)
     */
    Expression* evaluateExpression(const QString& command);

    /**
     * Interrupts all the running calculations in this session
     */
    virtual void interrupt() = 0;

    /**
     * Returns tab-completion, for this command/command-part.
     * The return type is a CompletionObject. The fetching
     * of the completions works asynchronously, you'll have to 
     * listen to the done() Signal of the returned object
     * @param cmd The partial command that should be completed
     * @return a Completion object, representing this completion
     * @see CompletionObject
     */
    virtual CompletionObject* completionFor(const QString& cmd);

    /**
     * Returns Syntax help, for this command.
     * It returns a SyntaxHelpObject, that will fetch the
     * needed information asynchroneousely. You need to listen
     * to the done() Signal of the Object
     * @param cmd the command, syntax help is requested for
     * @return SyntaxHelpObject, representing the help request
     * @see SyntaxHelpObject
     */
    virtual SyntaxHelpObject* syntaxHelpFor(const QString& cmd);

    /**
     * returns a syntax highlighter for this session
     * @param parent QTextEdit the Highlighter should be operating on
     * @return QSyntaxHighlighter doing the highlighting for this Session
     */
    virtual QSyntaxHighlighter* syntaxHighlighter(QTextEdit* parent);

    /**
     * returns a Model to interact with the variables
     * @return QAbstractItemModel to interact with the variables
     */
    virtual QAbstractItemModel* variableModel();

    /**
     * Enables/disables Typesetting for this session.
     * For this setting to make effect, the Backend must support
     * LaTeX typesetting (as indicated by the capabilities() flag.
     * @param enable true to enable, false to disable typesetting
     */
    virtual void setTypesettingEnabled(bool enable);

    /**
     * Returns the Backend, this Session is for
     * @return the Backend, this Session is for
     */
    Backend* backend();

    /**
     * Returns the status this Session has
     * @return the status this Session has
     */
    Cantor::Session::Status status();
    /**
     * Returns wether typesetting is enabled or not
     * @return wether typesetting is enabled or not
     */
    bool isTypesettingEnabled();
    
    /**
     * Returns the next available Expression id
     * It is basically a counter, incremented for
     * each new Expression
     * @return next Expression id
     */
    int nextExpressionId();

  protected:
    /**
     * Change the status of the Session. This will cause the
     * stausChanged signal to be emited
     * @param newStatus the new status of the session
     */
    void changeStatus(Cantor::Session::Status newStatus);

  signals:
    void statusChanged(Cantor::Session::Status newStatus);
    void ready();
    void error(const QString& msg);

  private:
    SessionPrivate* d;
};
}
#endif /* _SESSION_H */
