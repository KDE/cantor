/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _SYNTAXHELPOBJECT_H
#define _SYNTAXHELPOBJECT_H

#include <QObject>
#include "cantor_export.h"

namespace Cantor{

class SyntaxHelpObjectPrivate;
class Session;

/**
 * Object, used to display Syntax information to a given command
 * It is designed for asynchronous use. The Object emits done
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
    ~SyntaxHelpObject() override;

    /**
     * Start fetching the syntax help, emitting done() when done.
     */
    void fetchSyntaxHelp();

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

  Q_SIGNALS:
    /**
     * The SyntaxHelpObject is done, fetching the Information.
     * The syntax help can be shown now
     */
    void done();

  protected Q_SLOTS:
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
