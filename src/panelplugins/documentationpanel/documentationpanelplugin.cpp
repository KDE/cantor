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
    Copyright (C) 2020 Alexander Semke <alexander.semke@web.de>
 */

#include "cassert"

#include "documentationpanelplugin.h"
#include "session.h"

DocumentationPanelPlugin::DocumentationPanelPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent)
{
    Q_UNUSED(args);
}

DocumentationPanelPlugin::~DocumentationPanelPlugin()
{
}

QWidget* DocumentationPanelPlugin::widget()
{
    if(!m_widget)
    {
        m_widget = new DocumentationPanelWidget(parentWidget());
        connect(m_cantorShell, SIGNAL(requestDocumentation(QString)), m_widget, SLOT(contextSensitiveHelp(QString)));
    }

    return m_widget;
}

bool DocumentationPanelPlugin::showOnStartup()
{
    return true;
}

void DocumentationPanelPlugin::connectToShell(QObject* cantorShell)
{
    m_cantorShell = cantorShell;
    connect(cantorShell, SIGNAL(requestDocumentation(QString)), this, SIGNAL(visibilityRequested()));
}

Cantor::PanelPlugin::State DocumentationPanelPlugin::saveState()
{
    Cantor::PanelPlugin::State state = PanelPlugin::saveState();
    return state;
}

void DocumentationPanelPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    PanelPlugin::restoreState(state);
    if(session() && m_widget)
        m_widget->updateBackend(session()->backend()->name());
}

K_PLUGIN_FACTORY_WITH_JSON(documentationpanelplugin, "documentationpanelplugin.json", registerPlugin<DocumentationPanelPlugin>();)
#include "documentationpanelplugin.moc"
