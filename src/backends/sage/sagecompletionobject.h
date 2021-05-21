/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _SAGECOMPLETIONOBJECT_H
#define _SAGECOMPLETIONOBJECT_H

#include "completionobject.h"
#include "expression.h"

class SageSession;

class SageCompletionObject : public Cantor::CompletionObject
{
  Q_OBJECT
  public:
    SageCompletionObject( const QString& command, int index, SageSession* session);
    ~SageCompletionObject() override;

  protected:
    bool mayIdentifierContain(QChar c) const override;
    bool mayIdentifierBeginWith(QChar c) const override;


  protected Q_SLOTS:
    void fetchCompletions() override;
    void extractCompletions();
    void fetchIdentifierType() override;
    void extractIdentifierType(Cantor::Expression::Status status);

 private:
    void extractCompletionsNew();
    void extractCompletionsLegacy();

  private:
    Cantor::Expression* m_expression;
};

#endif /* _SAGECOMPLETIONOBJECT_H */
