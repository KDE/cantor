/*
    SPDX-FileCopyrightText: 2020-2023 by Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SAGESETTINGSWIDGET_H
#define SAGESETTINGSWIDGET_H

#include "ui_settings.h"
#include "../backendsettingswidget.h"

class SageSettingsWidget : public BackendSettingsWidget, public Ui::SageSettingsBase
{
Q_OBJECT

public:
    explicit SageSettingsWidget(QWidget* parent = nullptr, const QString& id = QString());

private Q_SLOTS:
    void integratePlotsChanged(bool);
};

#endif /* SAGESETTINGSWIDGET_H */
