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
    Copyright (C) 2019 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef _PYTHONVARIABLEMODEL_H
#define _PYTHONVARIABLEMODEL_H

#include <QStringList>

#include "defaultvariablemodel.h"

class JuliaSession;
class QDBusInterface;

class JuliaVariableModel : public Cantor::DefaultVariableModel
{
  Q_OBJECT
  public:
    JuliaVariableModel( JuliaSession* session);
    ~JuliaVariableModel() override = default;

    void update() override;

    void setJuliaServer(QDBusInterface* interface);

  private:
    static const QRegularExpression typeVariableInfo;

  private:
    QDBusInterface* m_interface;
    QStringList m_functions;
};

#endif /* _PYTHONVARIABLEMODEL_H */
