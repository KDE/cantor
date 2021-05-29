/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _MAXIMASYNTAXHELPOBJECT_H
#define _MAXIMASYNTAXHELPOBJECT_H

#include "syntaxhelpobject.h"

#include "expression.h"

class MaximaSession;

class MaximaSyntaxHelpObject : public Cantor::SyntaxHelpObject
{
  Q_OBJECT
  public:
    MaximaSyntaxHelpObject( const QString& command, MaximaSession* session );
    ~MaximaSyntaxHelpObject() override = default;

  protected Q_SLOTS:
    void fetchInformation() override;
  private Q_SLOTS:
    void expressionChangedStatus(Cantor::Expression::Status status);

  private:
    Cantor::Expression* m_expression;
};

#endif /* _MAXIMASYNTAXHELPOBJECT_H */
