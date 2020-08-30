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

#ifndef QTHELPCONFIG_H
#define QTHELPCONFIG_H

#include <QWidget>
#include <KNS3/Entry>

class QTreeWidgetItem;

namespace Ui
{
    class QtHelpConfigUI;
}

class QtHelpConfig : public QWidget
{
    Q_OBJECT

    public:
      explicit QtHelpConfig(const QString&);
      ~QtHelpConfig();

      bool checkNamespace(const QString &filename, QTreeWidgetItem* modifiedItem);

   Q_SIGNALS:
    void settingsChanged();

    private Q_SLOTS:
      void add();
      void remove(QTreeWidgetItem* item);
      void modify(QTreeWidgetItem* item);
      void knsUpdate(const KNS3::Entry::List& list);

    public Q_SLOTS:
      void loadSettings();
      void saveSettings();

    private:
      QTreeWidgetItem * addTableItem(const QString &icon, const QString &name,
                                     const QString &path, const QString &ghnsStatus);

      Ui::QtHelpConfigUI* m_configWidget;
      QString m_backend;
};

#endif // QTHELPCONFIG_H
