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

#include "cassert"

#include "documentationpanelplugin.h"
#include "session.h"

#include <QDebug>
#include <QIcon>

DocumentationPanelPlugin::DocumentationPanelPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent), m_widget(nullptr)
{
    Q_UNUSED(args);
}

DocumentationPanelPlugin::~DocumentationPanelPlugin()
{
    delete m_widget;
}

QWidget* DocumentationPanelPlugin::widget()
{
    //m_backendName = session()->backend()->name();
    //m_backendIcon = session()->backend()->icon();

    if(!m_widget)
    {
        m_widget = new DocumentationPanelWidget(QLatin1String("Maxima"), QLatin1String("maxima-backend"), parentWidget());
        //m_widget = new DocumentationPanelWidget(m_backendName, m_backendIcon, parentWidget());
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

void DocumentationPanelPlugin::connectToShell(QObject* cantorShell)
{
    connect(cantorShell, SIGNAL(requestDocumentation(QString)), m_widget, SLOT(contextSensitiveHelp(QString)));
    connect(cantorShell, SIGNAL(requestDocumentation(QString)), this, SIGNAL(visibilityRequested()));
}

Cantor::PanelPlugin::State DocumentationPanelPlugin::saveState()
{
    Cantor::PanelPlugin::State state = PanelPlugin::saveState();
    state.inners.append(m_backendName);
    state.inners.append(m_backendIcon);
    return state;
}

void DocumentationPanelPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    PanelPlugin::restoreState(state);

    if(state.inners.size() > 0)
    {
        assert(state.inners.size() == 2);
        m_backendName = state.inners[0].toString();
        m_backendIcon = state.inners[1].toString();

        /*if(m_widget)
            m_widget->updateBackend(m_backendName, m_backendIcon);*/
    }
    else if (session())
    {
        m_backendName = session()->backend()->name();
        m_backendIcon = session()->backend()->icon();

        qDebug() << m_backendName;
        m_widget = new DocumentationPanelWidget(m_backendName, m_backendIcon, parentWidget());
    }
}

K_PLUGIN_FACTORY_WITH_JSON(documentationpanelplugin, "documentationpanelplugin.json", registerPlugin<DocumentationPanelPlugin>();)
#include "documentationpanelplugin.moc"
