/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OCTAVECOMPLETIONOBJECT_H
#define OCTAVECOMPLETIONOBJECT_H

#include "completionobject.h"
#include "expression.h"

class OctaveCompletionObject : public Cantor::CompletionObject
{
    Q_OBJECT
public:
    OctaveCompletionObject(const QString& command, int index, Cantor::Session* parent);
    ~OctaveCompletionObject() override;

protected:
    void fetchCompletions() override;
    void fetchIdentifierType() override;
private Q_SLOTS:
    void extractCompletions(Cantor::Expression::Status status);
    void extractIdentifierType(Cantor::Expression::Status status);

    private:
    Cantor::Expression* m_expression;

};

#endif // OCTAVECOMPLETIONOBJECT_H
