/*
    SPDX-FileCopyrightText: 2020-2022 by Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2020-2022 by 2020 Shubham <aryan100jangid@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _MAXIMASETTINGSWIDGET_H
#define _MAXIMASETTINGSWIDGET_H

#include "ui_settings.h"
#include "../backendsettingswidget.h"

class MaximaSettingsWidget : public BackendSettingsWidget, public Ui::MaximaSettingsBase
{
Q_OBJECT

public:
    explicit MaximaSettingsWidget(QWidget* parent = nullptr, const QString& id = QString());

private Q_SLOTS:
    void integratePlotsChanged(bool);
};

#endif /* _MAXIMASETTINGSWIDGET_H */
