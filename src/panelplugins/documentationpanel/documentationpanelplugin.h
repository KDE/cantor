/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020-2021 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _DOCUMENTATIONPANELPLUGIN_H
#define _DOCUMENTATIONPANELPLUGIN_H

#include "panelplugin.h"

class DocumentationPanelWidget;

class DocumentationPanelPlugin : public Cantor::PanelPlugin
{
  Q_OBJECT
  public:
    DocumentationPanelPlugin(QObject* parent, QList<QVariant> args);
    ~DocumentationPanelPlugin() override;

    QWidget* widget() override;
    bool showOnStartup() override;
    void connectToShell(QObject* cantorShell) override;

    Cantor::PanelPlugin::State saveState() override;
    void restoreState(const Cantor::PanelPlugin::State&) override;

  private:
    DocumentationPanelWidget* m_widget = nullptr;
    QObject* m_cantorShell = nullptr;
};

#endif /* _DOCUMENTATIONPANELPLUGIN_H */
