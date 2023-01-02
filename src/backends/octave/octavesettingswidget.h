/*
    SPDX-FileCopyrightText: 2020-2022 by Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OCTAVESETTINGSWIDGET_H
#define OCTAVESETTINGSWIDGET_H

#include "ui_settings.h"
#include "../backendsettingswidget.h"

class OctaveSettingsWidget : public BackendSettingsWidget, public Ui::OctaveSettingsBase
{
  Q_OBJECT

public:
    explicit OctaveSettingsWidget(QWidget* parent = nullptr, const QString& id = QString());

private Q_SLOTS:
    void integratePlotsChanged(bool);
};

#endif /* OCTAVESETTINGSWIDGET_H */
