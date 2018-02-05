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
    Copyright (C) 2010 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _KALGEBRAEXTENSION_H
#define _KALGEBRAEXTENSION_H

#include "extension.h"

class KAlgebraVariableManagementExtension : public Cantor::VariableManagementExtension
{
  public:
    KAlgebraVariableManagementExtension( QObject* parent );
    ~KAlgebraVariableManagementExtension();

  public Q_SLOTS:
    QString addVariable(const QString& name, const QString& value) Q_DECL_OVERRIDE;
    QString setValue(const QString& name, const QString& value) Q_DECL_OVERRIDE;
    QString removeVariable(const QString&) Q_DECL_OVERRIDE { return QString(); }
    QString saveVariables(const QString&) Q_DECL_OVERRIDE { return QString(); }
    QString loadVariables(const QString&) Q_DECL_OVERRIDE { return QString(); }
    QString clearVariables() Q_DECL_OVERRIDE { return QString(); }
};

#endif /* _KALGEBRAEXTENSION_H */
