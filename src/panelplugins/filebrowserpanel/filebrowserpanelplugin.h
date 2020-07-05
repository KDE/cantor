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
    Copyright (C) 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _FILEBROWSERPANELPLUGIN_H
#define _FILEBROWSERPANELPLUGIN_H

#include "panelplugin.h"

class QTreeView;
class QModelIndex;
class QFileSystemModel;

class FileBrowserPanelPlugin : public Cantor::PanelPlugin
{
  Q_OBJECT
  public:
    FileBrowserPanelPlugin (QObject* parent, const QList<QVariant>& args);
    ~FileBrowserPanelPlugin() override;

    QWidget* widget() override;

    bool showOnStartup() override;

  Q_SIGNALS:
    void requestOpenWorksheet(const QUrl&);

  private Q_SLOTS:
    void handleDoubleClicked(const QModelIndex&);

  private:
    QPointer<QTreeView> m_view;
    QString m_dirRoot;
    static QFileSystemModel* model;
};

#endif /* _FILEBROWSERPANELPLUGIN_H */
