/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _PYTHONCOMPLETIONOBJECT_H
#define _PYTHONCOMPLETIONOBJECT_H

#include "completionobject.h"
#include "expression.h"

class PythonSession;

class PythonCompletionObject : public Cantor::CompletionObject
{
  public:
    PythonCompletionObject(const QString& cmd, int index, PythonSession* session);
    ~PythonCompletionObject() override;

  protected:
    bool mayIdentifierContain(QChar c) const override;
    bool mayIdentifierBeginWith(QChar c) const override;

  protected Q_SLOTS:
    void fetchCompletions() override;
    void fetchIdentifierType() override;
    void extractCompletions(Cantor::Expression::Status status);
    void extractIdentifierType(Cantor::Expression::Status status);

  private:
    Cantor::Expression* m_expression;
};

#endif /* _PYTHONCOMPLETIONOBJECT_H */
