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
    Copyright (C) 2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMAVARIABLEMODEL_H
#define _MAXIMAVARIABLEMODEL_H

#include "defaultvariablemodel.h"
#include <QStringList>

class MaximaSession;

class MaximaVariableModel : public Cantor::DefaultVariableModel
{
  Q_OBJECT
  public:
    static const QString inspectCommand;
    static const QString variableInspectCommand;

    explicit MaximaVariableModel( MaximaSession* session);
    ~MaximaVariableModel() override = default;

    void clearFunctions();

    QStringList functionNames(bool stripParameters=false);

    void update() override;

  private Q_SLOTS:
    void parseNewVariables(Cantor::Expression::Status status);
    void parseNewFunctions(Cantor::Expression::Status status);

  Q_SIGNALS:
    void functionsAdded(const QStringList funcs);
    void functionsRemoved(const QStringList funcs);

  private:
    MaximaSession* maximaSession();

  private:
    QStringList m_functions;
    Cantor::Expression* m_variableExpression;
    Cantor::Expression* m_functionExpression;
};

#endif /* _MAXIMAVARIABLEMODEL_H */
