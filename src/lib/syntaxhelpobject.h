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
 **/
class CANTOR_EXPORT SyntaxHelpObject : public QObject
{
  Q_OBJECT
  public:
    SyntaxHelpObject( const QString& command, Session* session );
    ~SyntaxHelpObject();

    QString toHtml();

    QString command();
    Session* session();

  signals:
    void done();

  protected slots:
    virtual void fetchInformation() = 0;

  protected:
    void setHtml(const QString& result);

  private:
    SyntaxHelpObjectPrivate* d;

};

}
#endif /* _SYNTAXHELPOBJECT_H */
