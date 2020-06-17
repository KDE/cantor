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
    Copyright (C) 2020 Shubham <aryan100jangid@gmail.com>
 */

#include "documentationpanelplugin.h"
#include "documentationpanelwidget.h"

DocumentationPanelPlugin::DocumentationPanelPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent), m_widget(nullptr)
{
    Q_UNUSED(args);
    m_showAtStart = true;
}

DocumentationPanelPlugin::~DocumentationPanelPlugin()
{
    delete m_widget;
}

void DocumentationPanelPlugin::onSessionChanged()
{
    if(m_widget)
        m_widget->setSession(session());
}

QWidget* DocumentationPanelPlugin::widget()
{
    if(m_widget == nullptr)
    {
        m_widget = new DocumentationPanelWidget(session(), parentWidget());
        //connect(m_widget.data(), &DocumentationPanelWidget::runCommand, this, &DocumentationPanelPlugin::requestRunCommand);
    }

    return m_widget;
}

bool DocumentationPanelPlugin::showOnStartup()
{
    return m_showAtStart;
}

void DocumentationPanelPlugin::setShowOnStartup(bool value)
{
    m_showAtStart = value;
}

K_PLUGIN_FACTORY_WITH_JSON(variablemanagerplugin, "documentationpanelplugin.json", registerPlugin<DocumentationPanelPlugin>();)
#include "documentationpanelplugin.moc"
