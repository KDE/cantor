/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
*/

#ifndef _RCOMPLETIONOBJECT_H
#define _RCOMPLETIONOBJECT_H

#include "completionobject.h"
#include <expression.h>

class RSession;

class RCompletionObject : public Cantor::CompletionObject
{
  Q_OBJECT
  public:
    RCompletionObject( const QString& cmd, int index, RSession* session );
    ~RCompletionObject() override;

  protected Q_SLOTS:
    void fetchCompletions() override;
    void receiveCompletions(Cantor::Expression::Status status);

  private:
    Cantor::Expression* m_expression;
};

#endif /* _RCOMPLETIONOBJECT_H */
