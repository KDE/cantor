/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef _PYTHONVARIABLEMODEL_H
#define _PYTHONVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class PythonSession;
class QDBusInterface;

class PythonVariableModel : public Cantor::DefaultVariableModel
{
  public:
    PythonVariableModel( PythonSession* session);
    ~PythonVariableModel() override;

    void update() override;

  private:
    Cantor::Expression* m_expression{nullptr};

  private Q_SLOTS:
    void extractVariables(Cantor::Expression::Status status);
};

#endif /* _PYTHONVARIABLEMODEL_H */
