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

#ifndef _SYNTAXHELPOBJECT_H
#define _SYNTAXHELPOBJECT_H

#include <QObject>
#include "cantor_export.h"

namespace Cantor{

class SyntaxHelpObjectPrivate;
class Session;

/**
 * Object, used to display Syntax informtion to a given command
 * It is designed for asynchroneous use. The Object emits done
 * as soon as fetching of the Help is finished
 *
 * @author Alexander Rieder
 **/
class CANTOR_EXPORT SyntaxHelpObject : public QObject
{
  Q_OBJECT
  public:
    /**
     * Construct a HelpObject, for the given command, belonging to the Session session.
     * @param command Command the help should be fetched for
     * @param session Session the HelpObject belongs to
     */
    SyntaxHelpObject( const QString& command, Session* session );
    /**
     * Destructor
     */
    ~SyntaxHelpObject();

    /**
     * Returns Html text of the Syntax Help
     */
    QString toHtml();

    /**
     * Returns the command, this SyntaxHelp is for
     * @return the command, this SyntaxHelp is for
     */
    QString command();
    /**
     * Returns the Session, this Object belongs to
     * @return the Session, this Object belongs to
     */
    Session* session();

  signals:
    /**
     * The SyntaxHelpObject is done, fetching the Information.
     * The syntax help can be shown now
     */
    void done();

  protected slots:
    /**
     * This method should fetch the Syntax help information from the backend
     */
    virtual void fetchInformation() = 0;

  protected:
    /**
     * Set the html syntax help
     * @param result the html syntax help
     */
    void setHtml(const QString& result);

  private:
    SyntaxHelpObjectPrivate* d;

};

}
#endif /* _SYNTAXHELPOBJECT_H */
