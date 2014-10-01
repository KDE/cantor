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

#include "variablemanagerplugin.h"

#include <QDebug>

#include "cantor_macros.h"
#include "session.h"
#include "variablemanagerwidget.h"

VariableManagerPlugin::VariableManagerPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent)
{
    Q_UNUSED(args);
    m_widget=0;

}

VariableManagerPlugin::~VariableManagerPlugin()
{
    delete m_widget;
}

void VariableManagerPlugin::onSessionChanged()
{
    if(m_widget)
        m_widget->setSession(session());
}

QWidget* VariableManagerPlugin::widget()
{
    if(m_widget==0)
    {
        qDebug()<<"creating new widget";
        m_widget=new VariableManagerWidget(session(), parentWidget());
        connect(m_widget, SIGNAL(runCommand(QString)), this, SIGNAL(requestRunCommand(QString)));
    }

    return m_widget;
}

Cantor::Backend::Capabilities VariableManagerPlugin::requiredCapabilities()
{
    return Cantor::Backend::VariableManagement;
}


#include "variablemanagerplugin.moc"

K_EXPORT_CANTOR_PLUGIN(variablemanagerplugin, VariableManagerPlugin)
