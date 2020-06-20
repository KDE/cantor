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

#include "helppanelplugin.h"

#include <KLocalizedString>
#include <KTextEdit>

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
        setHelpHtml(i18n("<h1>Cantor</h1>The KDE way to do Mathematics"));
        m_edit->setTextInteractionFlags(Qt::TextBrowserInteraction);

        //using old-style syntax here, otherwise we'd need to include and link to CantorPart and KParts
        connect(parent()->parent(), SIGNAL(showHelp(QString)), this, SLOT(setHelpHtml(QString)));
        connect(parent()->parent(), SIGNAL(showHelp(QString)), this, SIGNAL(visibilityRequested()));
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

K_PLUGIN_FACTORY_WITH_JSON(helppanelplugin, "helppanelplugin.json", registerPlugin<HelpPanelPlugin>();)
#include "helppanelplugin.moc"
