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
    Copyright (C) 2020-2024 Alexander Semke <alexander.semke@web.de>
 */

#include "backendsettingswidget.h"

#ifdef ENABLE_EMBEDDED_DOCUMENTATION
#include "qthelpconfig.h"
#endif

#include <QFile>
#include <QHBoxLayout>
#include <QTabWidget>

#include <KUrlRequester>

BackendSettingsWidget::BackendSettingsWidget(QWidget* parent, const QString& id) : QWidget(parent), m_id(id)
{

}

void BackendSettingsWidget::tabChanged(int index) {
    if (!m_tabWidget)
        return;

#ifdef ENABLE_EMBEDDED_DOCUMENTATION
    if (!m_tabDocumentation)
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
#endif
}

void BackendSettingsWidget::fileNameChanged(const QString& fileName) {
    if (!m_urlRequester)
        return;

    const bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
    if (invalid)
    {
        QPalette p;
        if (qGray(p.color(QPalette::Base).rgb()) > 160) /* light */
            m_urlRequester->setStyleSheet(QLatin1String("background: rgb(255, 200, 200);"));
        else /* dark */
            m_urlRequester->setStyleSheet(QLatin1String("background: rgb(128, 0, 0);"));
    }
    else
        m_urlRequester->setStyleSheet(QString());
}
