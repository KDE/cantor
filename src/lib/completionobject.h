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

#ifndef _COMPLETIONOBJECT_H
#define _COMPLETIONOBJECT_H

#include <kcompletion.h>

#include "cantor_export.h"

namespace Cantor
{
class CompletionObjectPrivate;
class Session;

/**
 * This Object is used to provide a Tab Completion, in an asynchroneous way.
 * Each backend, supporting tab completion, needs to provide their own
 * CompletionObject, that reimplements the fetching of the completions
 * and emits done() as soon as the completions are available
 * 
 * @author Alexander Rieder
 */
class CANTOR_EXPORT CompletionObject : public KCompletion
{
  Q_OBJECT
  public:
    /**
     * Constructor
     * @param parent the session, this object belongs to
     */
    CompletionObject(Session* parent);
    ///Destructor
    ~CompletionObject();

    enum LineCompletionMode {
	PreliminaryCompletion, ///< Only insert the completion
	FinalCompletion        ///< also add () for functions, etc
    };

   /**
     * Returns a list of completions
     * @return a list of completions
     */
    QStringList completions() const;
    /**
     * returns the command, this completion is for
     * @return the command, this completion is for
     */
    QString command() const;
    /**
     * returns the session, this object belongs to
     * @return the session, this object belongs to
     */
    Session* session() const;

    /**
     * Sets the line and cursor index at which a completion should be found
     * This triggers an asynchronous fetching of completions,
     * which emitts done() when done.
     * @param line the line that is to be completed
     * @param index the cursor postition in line
     */
    void setLine(const QString& line, int index);
    /**
     * Takes the changed line and updates the command accordingly.
     * This triggers an asynchronous fetching of completions,
     * which emitts done() when done.
     * @param line the line that is to be completed
     * @param index the cursor position in line
     */
    void updateLine(const QString& line, int index);
    /**
     * Takes a completion and a completion mode and triggers and calculates
     * the new line with this completion. If the completion mode is 
     * FinalCompletion some postprocessing is done asynchronously.
     * Emits lineDone when finished.
     * @param comp the completion that's to be processed
     * @param type whether the completion is final
     */
    void completeLine(const QString& comp, LineCompletionMode mode);

  protected:

    enum FunctionType {
	FunctionWithArguments,   ///< a function that takes arguments
	FunctionWithoutArguments ///< a function that takes no arguments
    };
    /**
     * returns the identifier for fetchIdentifierType
     * @return the identifier for fetchIdentifierType
     */
    QString identifier() const;

    /**
     * Sets the completions
     * @param completions list of possible completions
     */
    void setCompletions(const QStringList& completions);
    /**
     * sets the command/command-part
     * @param cmd the command/command-part
     */
    void setCommand(const QString& cmd);
    /**
     * Find an identifier in cmd that ends at index
     * @param cmd the command
     * @param index the index to look at
     */
    virtual int locateIdentifier(const QString& cmd, int index) const;
    /**
     * return true if c may be used in identifier names
     * @param c the character
     */
    virtual bool mayIdentifierContain(QChar c) const;
    /**
     * return true if identifier names can begin with c
     * @param c the character
     */
    virtual bool mayIdentifierBeginWith(QChar c) const;
    /**
     * Completes line with function identifier and emitts lineDone with the 
     * completed line. Helper function for completeLine.
     * @param type whether the function takes arguments, default: FunctionWithArguments
     */
    void completeFunctionLine(FunctionType type = FunctionWithArguments);
    /**
     * Completes line with keyword identifier and emitts lineDone with the 
     * completed line. Helper function for completeLine.
     */
    void completeKeywordLine();
    /**
     * Completes line with variable identifier and emitts lineDone with the 
     * completed line. Helper function for completeLine.
     */
    void completeVariableLine();
    /**
     * Completes line with identifier of unknown type and emitts lineDone with 
     * the completed line. Helper function for completeLine.
     */
    void completeUnknownLine();
  protected Q_SLOTS:
    /**
     * This function should be reimplemented to start the actual fetching
     * of the completions. It can be asynchroneous.
     * Rememver to emit done, if the fetching is complete
     */
    virtual void fetchCompletions() = 0;
    /**
     * Fetch the identifier type of d->commandCompletion, reimplement in
     * the backends. Call the appropriate complete*Line function when done.
     */
    virtual void fetchIdentifierType();
  Q_SIGNALS:
    /**
     * indicates that the fetching of completions is done, 
     * and that the completions can be used now
     */
    void done();
    /**
     * emitted when the line completion is done, passes the new line and
     * the cursor index
     */
    void lineDone(QString line, int index);
  private:
    CompletionObjectPrivate* d;
};

}

#endif /* _COMPLETIONOBJECT_H */
