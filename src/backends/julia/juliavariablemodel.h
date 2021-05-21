/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019 Nikita Sirgienko <warquark@gmail.com>
*/

#ifndef _JULIAVARIABLEMODEL_H
#define _JULIAVARIABLEMODEL_H

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
    static const QStringList internalCantorJuliaVariables;

  private:
    QDBusInterface* m_interface;
    QStringList m_functions;
};

#endif /* _JULIAVARIABLEMODEL_H */
