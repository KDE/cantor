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

DocumentationPanelPlugin::DocumentationPanelPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent), m_widget(nullptr), m_showAtStart(true)
{
    Q_UNUSED(args);
}

DocumentationPanelPlugin::~DocumentationPanelPlugin()
{
    delete m_widget;
}

QWidget* DocumentationPanelPlugin::widget()
{
    if(!m_widget)
    {
        m_widget = new DocumentationPanelWidget(parentWidget());
        connect(parent()->parent(), SIGNAL(requestDocumentation(QString)), m_widget, SLOT(contextSensitiveHelp(QString)));
        connect(parent()->parent(), SIGNAL(requestDocumentation(QString)), this, SIGNAL(visibilityRequested()));
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
