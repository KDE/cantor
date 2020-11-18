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
    Copyright (C) 2020 Alexander Semke <alexander.semke@web.de>
 */

#include "backendsettingswidget.h"
#include "qthelpconfig.h"

#include <QHBoxLayout>
#include <QTabWidget>

BackendSettingsWidget::BackendSettingsWidget(QWidget* parent, const QString& id) : QWidget(parent), m_id(id)
{

}

void BackendSettingsWidget::tabChanged(int index) {
    if (!m_tabWidget || !m_tabDocumentation)
        return;

    //if the documentation tab was selected and there is not doc widget available yet, create it
    if (m_tabWidget->widget(index) == m_tabDocumentation)
    {
        if (!m_docWidget)
        {
            m_docWidget = new QtHelpConfig(m_id);
            auto hboxLayout = new QHBoxLayout(m_tabDocumentation);
            hboxLayout->addWidget(m_docWidget);
        }
    }
}
