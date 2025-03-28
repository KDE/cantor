/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
    SPDX-FileCopyrightText: 2020-2025 by Alexander Semke (alexander.semke@web.de)
*/

#ifndef _RSETTINGSWIDGET_H
#define _RSETTINGSWIDGET_H

#include "ui_settings.h"
#include "../backendsettingswidget.h"

class RSettingsWidget : public BackendSettingsWidget, public Ui::RSettingsBase
{
Q_OBJECT

public:
    explicit RSettingsWidget(QWidget* parent = nullptr, const QString& id = QString());
    bool eventFilter(QObject*, QEvent*) override;

public Q_SLOTS:
    void integratePlotsChanged(bool);
    void displayFileSelectionDialog();
};

#endif /* _RSETTINGSWIDGET_H */
