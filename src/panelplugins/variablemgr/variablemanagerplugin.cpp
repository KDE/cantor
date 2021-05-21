/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "variablemanagerplugin.h"

#include "session.h"
#include "variablemanagerwidget.h"

VariableManagerPlugin::VariableManagerPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent), m_widget(nullptr)
{
    Q_UNUSED(args);
}

VariableManagerPlugin::~VariableManagerPlugin()
{
    delete m_widget;
}

void VariableManagerPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    PanelPlugin::restoreState(state);
    if(m_widget)
        m_widget->setSession(session());
}

QWidget* VariableManagerPlugin::widget()
{
    if(m_widget==nullptr)
    {
        m_widget=new VariableManagerWidget(session(), parentWidget());
        connect(m_widget.data(), &VariableManagerWidget::runCommand, this, &VariableManagerPlugin::requestRunCommand);
    }

    return m_widget;
}

Cantor::Backend::Capabilities VariableManagerPlugin::requiredCapabilities()
{
    return Cantor::Backend::VariableManagement;
}

K_PLUGIN_FACTORY_WITH_JSON(variablemanagerplugin, "variablemanagerplugin.json", registerPlugin<VariableManagerPlugin>();)
#include "variablemanagerplugin.moc"
