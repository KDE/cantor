/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _KALGEBRAEXTENSION_H
#define _KALGEBRAEXTENSION_H

#include "extension.h"

class KAlgebraVariableManagementExtension : public Cantor::VariableManagementExtension
{
  public:
    explicit KAlgebraVariableManagementExtension( QObject* parent );
    ~KAlgebraVariableManagementExtension() override = default;

  public Q_SLOTS:
    QString addVariable(const QString& name, const QString& value) override;
    QString setValue(const QString& name, const QString& value) override;
    QString removeVariable(const QString&) override { return QString(); }
    QString saveVariables(const QString&) override { return QString(); }
    QString loadVariables(const QString&) override { return QString(); }
    QString clearVariables() override { return QString(); }
};

#endif /* _KALGEBRAEXTENSION_H */
