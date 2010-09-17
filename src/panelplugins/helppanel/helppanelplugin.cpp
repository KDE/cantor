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

#include <ktextedit.h>
#include "cantor_macros.h"

HelpPanelPlugin::HelpPanelPlugin(QObject* parent, QList<QVariant> args) : Cantor::PanelPlugin(parent)
{
    Q_UNUSED(args);
    m_edit=0;
}

HelpPanelPlugin::~HelpPanelPlugin()
{
    delete m_edit;
}

QWidget* HelpPanelPlugin::widget()
{
    if(m_edit==0)
    {
        m_edit=new KTextEdit(parentWidget());
        m_edit->setText(i18n("<h1>Cantor</h1>The KDE way to do Mathematics"));
        m_edit->setTextInteractionFlags(Qt::TextBrowserInteraction);

        connect(parent()->parent(), SIGNAL(showHelp(QString)), m_edit, SLOT(setHtml(QString)));
    }

    return m_edit;
}

void HelpPanelPlugin::showHelp(const QString& help)
{
    if(m_edit)
        m_edit->setHtml(help);
}

#include "helppanelplugin.moc"

K_EXPORT_CANTOR_PLUGIN(helppanelplugin, HelpPanelPlugin)
