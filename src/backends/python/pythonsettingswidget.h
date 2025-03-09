/*
    SPDX-FileCopyrightText: 2020-2025 by Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
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

private Q_SLOTS:
    void integratePlotsChanged(bool);
};

#endif /* _PYTHONSETTINGSWIDGET_H */
