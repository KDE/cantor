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


#include <QTreeWidgetItem>

#include "maximasettingswidget.h"
#include "../qthelpconfig.h"

#include <KConfigGroup>
#include <KSharedConfig>

MaximaSettingsWidget::MaximaSettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    // Add QtHelp widget
    /*QtHelpConfig**/ docWidget = new QtHelpConfig();
    static_cast<QGridLayout*>(this->layout())->addWidget(docWidget, 6, 0, 1, 3);
}

MaximaSettingsWidget::~MaximaSettingsWidget()
{

    /*KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Documentation"));

    QString name;
    QString path;

    for (int i = 0; i < docWidget->qchTable->topLevelItemCount(); i++)
    {
        const QTreeWidgetItem* item = docWidget->qchTable->topLevelItem(i);
        name = item->text(0);
        path = item->text(1);
    }

    group.writeEntry(QLatin1String("Name"), name);
    group.writeEntry(QLatin1String("Path"), path);*/
}
