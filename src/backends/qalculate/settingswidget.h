/*
    Copyright (C) 2011  Matteo Agostinelli <agostinelli@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "ui_settings.h"

class QalculateSettingsWidget : public QWidget, public Ui::QalculateSettingsBase
{
Q_OBJECT

public:
    explicit QalculateSettingsWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
};

#endif