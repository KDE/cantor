/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef _OCTAVEVARIABLEMODEL_H
#define _OCTAVEVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class OctaveSession;

class OctaveVariableModel : public Cantor::DefaultVariableModel
{
  public:
    OctaveVariableModel( OctaveSession* session);
    ~OctaveVariableModel() override = default;

    void update() override;

  private Q_SLOTS:
    void parseNewVariables(Cantor::Expression::Status status);

  private:
    Cantor::Expression* m_expr;
};

#endif /* _OCTAVEVARIABLEMODEL_H */
