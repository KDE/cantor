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

#ifndef _TABCOMPLETIONOBJECT_H
#define _TABCOMPLETIONOBJECT_H

#include <kcompletion.h>

#include "cantor_export.h"

namespace Cantor
{
class TabCompletionObjectPrivate;
class Session;

/**
 * This Object is used to provide a Tab Completion, in an asynchroneous way.
 * Each backend, supporting tab completion, needs to provide their own
 * TabCompletionObject, that reimplements the fetching of the completions
 * and emits done() as soon as the completions are available
 * 
 * @author Alexander Rieder
 */
class CANTOR_EXPORT TabCompletionObject : public KCompletion
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
    TabCompletionObject( const QString& command, Session* parent);
    ///Destrutctor
    ~TabCompletionObject();

    /**
     * Returns a list of completions
     * @return a list of completions
     */
    QStringList completions();
    /**
     * returns the command, this completion is for
     * @return the command, this completion is for
     */
    QString command();
    /**
     * returns the sessiion, this object belongs to
     * @return the sessiion, this object belongs to
     */
    Session* session();
  protected:
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
    TabCompletionObjectPrivate* d;
};

}

#endif /* _TABCOMPLETIONOBJECT_H */
