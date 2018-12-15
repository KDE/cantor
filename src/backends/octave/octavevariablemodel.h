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
    Copyright (C) 2018 Nikita Sirgienko <warquark@gmail.com>
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
