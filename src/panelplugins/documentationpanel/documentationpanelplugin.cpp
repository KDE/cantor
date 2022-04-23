/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>
*/

#include "documentationpanelplugin.h"
#include "documentationpanelwidget.h"
#include "session.h"

#include <KPluginFactory>

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
    state.inners.append(m_widget->url()); //save the currently shown URL in the web view
    return state;
}

void DocumentationPanelPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    PanelPlugin::restoreState(state);

    //TODO: when using this panel in LabPlot this function is being called before widget().
    //the reason is not completely clear. call widget() here to make sure it's available.
    if (!m_widget)
        this->widget();

    if(session() && m_widget)
    {
        m_widget->updateBackend(session()->backend()->name());
        if (state.inners.size() == 1)
            m_widget->showUrl(state.inners.first().toUrl());
    }
}

K_PLUGIN_FACTORY_WITH_JSON(documentationpanelplugin, "documentationpanelplugin.json", registerPlugin<DocumentationPanelPlugin>();)
#include "documentationpanelplugin.moc"
