/*
    SPDX-FileCopyrightText: 2020-2025 by Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JULIASETTINGSWIDGET_H
#define JULIASETTINGSWIDGET_H

#include "ui_settings.h"
#include "../backendsettingswidget.h"

class JuliaSettingsWidget : public BackendSettingsWidget, public Ui::JuliaSettingsBase
{
  Q_OBJECT

public:
    explicit JuliaSettingsWidget(QWidget* parent = nullptr, const QString& id = QString());

private Q_SLOTS:
    void integratePlotsChanged(bool);
};

#endif /* JULIASETTINGSWIDGET_H */
