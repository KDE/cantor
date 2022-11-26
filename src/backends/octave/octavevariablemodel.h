/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _OCTAVEVARIABLEMODEL_H
#define _OCTAVEVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class OctaveSession;

class OctaveVariableModel : public Cantor::DefaultVariableModel
{
  public:
    OctaveVariableModel(OctaveSession*);
    ~OctaveVariableModel() override = default;

    void update() override;

  private Q_SLOTS:
    void parseNewVariables(Cantor::Expression::Status);

  private:
    Cantor::Expression* m_expr = nullptr;
};

#endif /* _OCTAVEVARIABLEMODEL_H */
