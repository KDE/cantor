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
    Copyright (C) 2020-2022 Alexander Semke <alexander.semke@web.de>
 */

#include "sagesettingswidget.h"
#include <QTimer>

SageSettingsWidget::SageSettingsWidget(QWidget* parent, const QString& id) : BackendSettingsWidget(parent, id)
{
    setupUi(this);

    m_tabWidget = tabWidget;
    m_tabDocumentation = tabDocumentation;
    m_urlRequester = kcfg_Path;

    connect(tabWidget, &QTabWidget::currentChanged, this, &BackendSettingsWidget::tabChanged);
    connect(kcfg_Path, &KUrlRequester::textChanged, this, &BackendSettingsWidget::fileNameChanged);
    connect(kcfg_integratePlots, &QCheckBox::clicked, this, &SageSettingsWidget::integratePlotsChanged);

    kcfg_inlinePlotFormat->setItemIcon(0, QIcon::fromTheme(QLatin1String("application-pdf")));
    kcfg_inlinePlotFormat->setItemIcon(1, QIcon::fromTheme(QLatin1String("image-png")));

    // return to the event loop to show the settings
    // and then call the slot to update the state of the widgets
    QTimer::singleShot(0, this, [=]() {
        integratePlotsChanged(kcfg_integratePlots->isChecked());
    });
}

void SageSettingsWidget::integratePlotsChanged(bool state)
{
    lPlotWidth->setEnabled(state);
    kcfg_plotWidth->setEnabled(state);
    lPlotHeight->setEnabled(state);
    kcfg_plotHeight->setEnabled(state);
    kcfg_inlinePlotFormat->setEnabled(state);
}
