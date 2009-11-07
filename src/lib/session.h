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

namespace Cantor
{
class Backend;
class SessionPrivate;
class TabCompletionObject;
class SyntaxHelpObject;

class CANTOR_EXPORT Session : public QObject
{
  Q_OBJECT
  public:
    enum Status { Running, Done };
    Session( Backend* backend);
    ~Session();

    virtual void login() = 0;
    virtual void logout() = 0;
    
    /** Passes the given command to the backend and returns a Pointer 
     *  to a new Expression object, which will emit the resultArrived() 
     *  signal as soon as the computation is done. The result will 
     *  then be acessible by Expression::result()
     */
    virtual Expression* evaluateExpression(const QString& command, Expression::FinishingBehavior finishingBehavior) = 0;
    Expression* evaluateExpression(const QString& command);

    /** Interrupts all the running calculations in this session **/
    virtual void interrupt() = 0;

    /** Returns tab-completion, for this command/command-part.
	The return type is a TabCompletionObject. The fetching
	of the completions works asynchronously, you'll have to 
	listen to the done() Signal of the returned object
     **/
    virtual TabCompletionObject* tabCompletionFor(const QString& cmd);

    /** Returns Syntax help, for this command.
	It returns a SyntaxHelpObject, that will fetch the
	needed information asynchrneousely. You need to listen
	to the done() Signal of the Object
    **/
    virtual SyntaxHelpObject* syntaxHelpFor(const QString& cmd);

    /**returns a syntax highlighter for this session **/
    virtual QSyntaxHighlighter* syntaxHighlighter(QTextEdit* parent);

    virtual void setTypesettingEnabled(bool enable);

    Backend* backend();

    Cantor::Session::Status status();
    bool isTypesettingEnabled();
    
    int nextExpressionId();

  protected:
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
