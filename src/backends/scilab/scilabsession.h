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
    Copyright (C) 2011 Filipe Saraiva <filip.saraiva@gmail.com>
 */

#ifndef _SCILABSESSION_H
#define _SCILABSESSION_H

#include "session.h"


class ScilabExpression;

class ScilabSession : public Cantor::Session
{
  Q_OBJECT
  public:
    ScilabSession( Cantor::Backend* backend);
    ~ScilabSession();

    void login();
    void logout();

    void interrupt();

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave);
//     Cantor::CompletionObject* completionFor(const QString& cmd);

  private slots:
    void expressionFinished();

  private:
    QList<ScilabExpression*> m_runningExpressions;
};

#endif /* _SCILABSESSION_H */
