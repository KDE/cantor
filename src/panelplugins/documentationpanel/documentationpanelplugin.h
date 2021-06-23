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
    Copyright (C) 2020 Shubham <aryan100jangid@gmail.com>
    Copyright (C) 2020 Alexander Semke <alexander.semke@web.de>
 */

#ifndef _DOCUMENTATIONPANELPLUGIN_H
#define _DOCUMENTATIONPANELPLUGIN_H

#include "documentationpanelwidget.h"
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
    void restoreState(const Cantor::PanelPlugin::State& state) override;

  private:
    DocumentationPanelWidget* m_widget = nullptr;
    QObject* m_cantorShell = nullptr;
};

#endif /* _DOCUMENTATIONPANELPLUGIN_H */
