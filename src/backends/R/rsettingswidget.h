/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
*/

#ifndef _RSETTINGSWIDGET_H
#define _RSETTINGSWIDGET_H

#include "ui_settings.h"

class RSettingsWidget : public QWidget,public Ui::RSettingsBase
{
  Q_OBJECT

  public:
    explicit RSettingsWidget(QWidget *parent = nullptr);
    bool eventFilter(QObject*, QEvent*) override;

  public Q_SLOTS:
    void displayFileSelectionDialog();
};

#endif /* _RSETTINGSWIDGET_H */
