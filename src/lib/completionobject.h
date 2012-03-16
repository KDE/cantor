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
     * @param command command-part, that should be completed (this always contains the whole line entered in
     *                 the worksheet. It's up to the CompletionObject to decide up to where the command is useful
     *                 for completion
     * @param parent the session, this object belongs to
     */
    CompletionObject( const QString& command, int index, Session* parent);
    ///Destrutctor
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
     * Takes a completion and returns the complete line with this completion
     * inserted and the index for the new cursor position. If type is 
     * FinalCompletion some postprocessing is done, like adding () for 
     * functions.
     * @param comp the completion that's to be processed
     * @param type whether the completion is final
     * @return QPair containing the completed line and the cursor position
     */
    virtual QPair<QString, int> completeLine(const QString& comp, LineCompletionMode mode);
  protected:

    enum FunctionType {
	HasNoArguments, ///< The function does not take arguments
	HasArguments    ///< The function may take arguments
    };
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
     * Takes a function completion and returns the completed line. 
     * Helper function for completeLine.
     * @param func the completion that's to be processed
     * @param type whether the function takes arguments, default: HasArguments
     * @return QPair containing the processed completion and the cursor offset
     */
    QPair<QString, int> completeFunctionLine(const QString& func, FunctionType type = HasArguments) const;
    /**
     * Takes a keyword completion and returns the completed line.
     * Helper function for completeLine.
     * @param keyword the completion that's to be processed
     * @return QPair containing the processed completion and the cursor offset
     */
    QPair<QString, int> completeKeywordLine(const QString& keyword) const;
    /**
     * Takes a variable completion and returns the completed line.
     * Helper function for completeLine.
     * @param var the completion that's to be processed
     * @return QPair containing the processed completion and the cursor offset
     */
    QPair<QString, int> completeVariableLine(const QString& var) const;
  protected Q_SLOTS:
    /**
     * This function should be reimplemented to start the actual fetching
     * of the completions. It can be asynchroneous.
     * Rememver to emit done, if the fetching is complete
     */
    virtual void fetchCompletions() = 0;
  Q_SIGNALS:
    /**
     * indicates that the fetching of completions is done, 
     * and that the completions can be used now
     */
    void done();
  private:
    CompletionObjectPrivate* d;
};

}

#endif /* _COMPLETIONOBJECT_H */
