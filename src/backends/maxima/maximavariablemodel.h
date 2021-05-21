/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _MAXIMAVARIABLEMODEL_H
#define _MAXIMAVARIABLEMODEL_H

#include "defaultvariablemodel.h"
#include <QStringList>

class MaximaSession;
class MaximaExpression;

class MaximaVariableModel : public Cantor::DefaultVariableModel
{
  Q_OBJECT
  public:
    static const QString inspectCommand;
    static const QString variableInspectCommand;

    explicit MaximaVariableModel( MaximaSession* session);
    ~MaximaVariableModel() override = default;

    void update() override;

  private Q_SLOTS:
    void parseNewVariables(Cantor::Expression::Status status);
    void parseNewFunctions(Cantor::Expression::Status status);

  private:
    MaximaSession* maximaSession();

  private:
    MaximaExpression* m_variableExpression;
    MaximaExpression* m_functionExpression;
};

#endif /* _MAXIMAVARIABLEMODEL_H */
