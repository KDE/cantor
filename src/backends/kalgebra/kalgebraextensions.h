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
    virtual QString addVariable(const QString& name, const QString& value);
    virtual QString setValue(const QString& name,const QString& value);
    virtual QString removeVariable(const QString& name) { return QString(); }
    virtual QString saveVariables(const QString& fileName) { return QString(); }
    virtual QString loadVariables(const QString& fileName) { return QString(); }
    virtual QString clearVariables() { return QString(); }
};

#endif /* _KALGEBRAEXTENSION_H */
