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

#ifndef PYTHONSETTINGSWIDGET_H
#define PYTHONSETTINGSWIDGET_H

#include "ui_settings.h"
#include "../backendsettingswidget.h"

class PythonSettingsWidget : public BackendSettingsWidget, public Ui::PythonSettingsBase
{
  Q_OBJECT

  public:
    explicit PythonSettingsWidget(QWidget* parent = nullptr, const QString& id = QString());
};

#endif /* _PYTHONSETTINGSWIDGET_H */
