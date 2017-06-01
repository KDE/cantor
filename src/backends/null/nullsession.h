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

#ifndef _NULLSESSION_H
#define _NULLSESSION_H

#include "session.h"


class NullExpression;

class NullSession : public Cantor::Session
{
  Q_OBJECT
  public:
    NullSession( Cantor::Backend* backend);
    ~NullSession();

    void login() Q_DECL_OVERRIDE;
    void logout() Q_DECL_OVERRIDE;

    void interrupt() Q_DECL_OVERRIDE;

    Cantor::Expression* evaluateExpression(const QString& command, Cantor::Expression::FinishingBehavior behave) Q_DECL_OVERRIDE;
    Cantor::CompletionObject* completionFor(const QString& cmd, int index=-1) Q_DECL_OVERRIDE;

  private Q_SLOTS:
    void expressionFinished();

  private:
    QList<NullExpression*> m_runningExpressions;
};

#endif /* _NULLSESSION_H */
