/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _VARIABLEMANAGERPLUGIN_H
#define _VARIABLEMANAGERPLUGIN_H

#include <QPointer>

#include "panelplugin.h"

class VariableManagerWidget;

class VariableManagerPlugin : public Cantor::PanelPlugin
{
  Q_OBJECT
  public:
    VariableManagerPlugin( QObject* parent, QList<QVariant> args);
    ~VariableManagerPlugin() override;

    QWidget* widget() override;

    Cantor::Backend::Capabilities requiredCapabilities() override;

    void restoreState(const Cantor::PanelPlugin::State & state) override;

  private:
    QPointer<VariableManagerWidget> m_widget;

};

#endif /* _VARIABLEMANAGERPLUGIN_H */
