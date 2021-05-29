/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OCTAVESYNTAXHELPOBJECT_H
#define OCTAVESYNTAXHELPOBJECT_H

#include <syntaxhelpobject.h>
#include <expression.h>

class OctaveSyntaxHelpObject : public Cantor::SyntaxHelpObject
{
  Q_OBJECT
  public:
    OctaveSyntaxHelpObject(const QString& command, Cantor::Session* session);
    ~OctaveSyntaxHelpObject() override = default;

protected:
    void fetchInformation() override;

  private Q_SLOTS:
    void fetchingDone(Cantor::Expression::Status status);

  private:
    Cantor::Expression* m_expression;
};

#endif // OCTAVESYNTAXHELPOBJECT_H
