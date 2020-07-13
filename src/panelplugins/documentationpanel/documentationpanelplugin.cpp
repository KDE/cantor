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
#include "session.h"

#include <QIcon>

DocumentationPanelPlugin::DocumentationPanelPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent), m_widget(nullptr)
{
    Q_UNUSED(args);
}

DocumentationPanelPlugin::~DocumentationPanelPlugin()
{
    delete m_widget;
}

void DocumentationPanelPlugin::onSessionChanged()
{
    if(m_widget)
    {
        m_widget->setBackend(m_backendName);
        m_widget->setBackendIcon(m_backendIcon);
    }
}

QWidget* DocumentationPanelPlugin::widget()
{
    m_backendName = session()->backend()->name();
    m_backendIcon = session()->backend()->icon();

    if(!m_widget)
    {
        m_widget = new DocumentationPanelWidget(m_backendName, m_backendIcon, parentWidget());
        connect(parent()->parent(), SIGNAL(requestDocumentation(QString)), m_widget, SLOT(contextSensitiveHelp(QString)));
        connect(parent()->parent(), SIGNAL(requestDocumentation(QString)), this, SIGNAL(visibilityRequested()));
    }

    return m_widget;
}

bool DocumentationPanelPlugin::showOnStartup()
{
    return true;
}

QIcon DocumentationPanelPlugin::icon() const
{
    return QIcon::fromTheme(m_backendIcon);
}

QString DocumentationPanelPlugin::backendName() const
{
    return m_backendName;
}

K_PLUGIN_FACTORY_WITH_JSON(documentationpanelplugin, "documentationpanelplugin.json", registerPlugin<DocumentationPanelPlugin>();)
#include "documentationpanelplugin.moc"
