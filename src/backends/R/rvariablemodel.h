/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef _RVARIABLEMODEL_H
#define _RVARIABLEMODEL_H

#include "defaultvariablemodel.h"

class RSession;

class RVariableModel : public Cantor::DefaultVariableModel
{
  Q_OBJECT
  public:
    RVariableModel(RSession*);
    ~RVariableModel() override;

    // List of virables from other R namespaces (packages), which can be treted as constants, like "pi"
    //QStringList constants() const;

    void update() override;

  Q_SIGNALS:
    void constantsAdded(QStringList);
    void constantsRemoved(QStringList);

  public Q_SLOTS:
    void parseResult(Cantor::Expression::Status);

  private:
    void setConstants(QStringList);

  private:
    QStringList m_constants;
    Cantor::Expression* m_expression{nullptr};
};

#endif /* _RVARIABLEMODEL_H */
