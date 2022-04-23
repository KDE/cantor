/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "helppanelplugin.h"

#include <KLocalizedString>
#include <KTextEdit>
#include <KPluginFactory>

HelpPanelPlugin::HelpPanelPlugin(QObject* parent, const QList<QVariant>& args) : Cantor::PanelPlugin(parent), m_edit(nullptr)
{
    Q_UNUSED(args);
}

HelpPanelPlugin::~HelpPanelPlugin()
{
    delete m_edit;
}

QWidget* HelpPanelPlugin::widget()
{
    if(!m_edit)
    {
        m_edit = new KTextEdit(parentWidget());
        setHelpHtml(QString());
        m_edit->setTextInteractionFlags(Qt::TextBrowserInteraction);
    }

    return m_edit;
}

void HelpPanelPlugin::setHelpHtml(const QString& help)
{
    if(!m_edit)
        return;

    m_edit->setHtml(help);
    m_edit->selectAll();
    m_edit->setFontFamily(QLatin1String("Monospace"));
    m_edit->moveCursor(QTextCursor::Start);
}

void HelpPanelPlugin::showHelp(const QString& help)
{
    if(m_edit)
        m_edit->setHtml(help);
}

bool HelpPanelPlugin::showOnStartup()
{
    return false;
}

void HelpPanelPlugin::connectToShell(QObject* cantorShell)
{
    //using old-style syntax here, otherwise we'd need to include and link to CantorPart and KParts
    connect(cantorShell, SIGNAL(showHelp(QString)), this, SLOT(setHelpHtml(QString)));
    connect(cantorShell, SIGNAL(showHelp(QString)), this, SIGNAL(visibilityRequested()));
}

Cantor::PanelPlugin::State HelpPanelPlugin::saveState()
{
    auto state = PanelPlugin::saveState();
    state.inners.append(m_edit->toHtml());
    return state;
}

void HelpPanelPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    PanelPlugin::restoreState(state);
    if(state.inners.size() > 0)
        setHelpHtml(state.inners.first().toString());
    else
        setHelpHtml(QString());
}

K_PLUGIN_FACTORY_WITH_JSON(helppanelplugin, "helppanelplugin.json", registerPlugin<HelpPanelPlugin>();)
#include "helppanelplugin.moc"
