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

    /** @return icon of the current backend **/
    QIcon icon() const;

    /** @return name of the current backend **/
    QString backendName() const;

  private:
    QPointer<DocumentationPanelWidget> m_widget;
    QString m_backendName;
    QString m_backendIcon;
};

#endif /* _DOCUMENTATIONPANELPLUGIN_H */
