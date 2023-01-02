/*
    SPDX-FileCopyrightText: 2020-2022 by Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "octavesettingswidget.h"
#include <QTimer>

OctaveSettingsWidget::OctaveSettingsWidget(QWidget* parent, const QString& id) : BackendSettingsWidget(parent, id)
{
    setupUi(this);

    m_tabWidget = tabWidget;
    m_tabDocumentation = tabDocumentation;
    m_urlRequester = kcfg_Path;

    connect(tabWidget, &QTabWidget::currentChanged, this, &BackendSettingsWidget::tabChanged);
    connect(kcfg_Path, &KUrlRequester::textChanged, this, &BackendSettingsWidget::fileNameChanged);
    connect(kcfg_integratePlots, &QCheckBox::clicked, this, &OctaveSettingsWidget::integratePlotsChanged);

    kcfg_inlinePlotFormat->setItemIcon(0, QIcon::fromTheme(QLatin1String("application-pdf")));
    kcfg_inlinePlotFormat->setItemIcon(1, QIcon::fromTheme(QLatin1String("image-svg+xml")));
    kcfg_inlinePlotFormat->setItemIcon(2, QIcon::fromTheme(QLatin1String("image-png")));

    // return to the event loop to show the settings
    // and then call the slot to update the state of the widgets
    QTimer::singleShot(0, this, [=]() {
        integratePlotsChanged(kcfg_integratePlots->isChecked());
    });
}

void OctaveSettingsWidget::integratePlotsChanged(bool state)
{
    lPlotWidth->setEnabled(state);
    kcfg_plotWidth->setEnabled(state);
    lPlotHeight->setEnabled(state);
    kcfg_plotHeight->setEnabled(state);
    kcfg_inlinePlotFormat->setEnabled(state);
}
