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

#include "maximasettingswidget.h"
#include "../qthelpconfig.h"
#include <QDebug>
#include <KConfigGroup>
#include <KSharedConfig>

MaximaSettingsWidget::MaximaSettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    // Add QtHelp widget
    QtHelpConfig* docWidget = new QtHelpConfig(QLatin1String("maxima"));
    static_cast<QGridLayout*>(this->layout())->addWidget(docWidget, 6, 0, 1, 3);

    loadSettings(); // load previously saved settings from read KConfig
}

void MaximaSettingsWidget::loadSettings()
{
    const KConfigGroup cg = KSharedConfig::openConfig()->group(QLatin1String("Settings_Documentation"));

    QStringList nameList = cg.readEntry("Names", QStringList());
    QStringList pathList = cg.readEntry("Paths", QStringList());

    qDebug() << nameList.at(0);
    /*ui.chkShowColumnType->setChecked(group.readEntry(QLatin1String("Names"), QStringList()));
    ui.chkShowPlotDesignation->setChecked(group.readEntry(QLatin1String("Paths"), QStringList()));*/
}
