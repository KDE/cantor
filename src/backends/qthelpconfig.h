/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
 */

#ifndef QTHELPCONFIG_H
#define QTHELPCONFIG_H

#include <QWidget>
#include <KNS3/Entry>

class QTreeWidget;
class QTreeWidgetItem;

class QtHelpConfig : public QWidget
{
    Q_OBJECT

    public:
      explicit QtHelpConfig(const QString&);
      ~QtHelpConfig();

      bool checkNamespace(const QString& filename, QTreeWidgetItem* modifiedItem);

   Q_SIGNALS:
    void settingsChanged();

    private Q_SLOTS:
      void add();
      void remove(QTreeWidgetItem*);
      void modify(QTreeWidgetItem*);
      void knsUpdate(const KNS3::Entry::List&);
      void saveSettings();

    private:
      void loadSettings();
      QTreeWidgetItem* addTableItem(const QString& icon, const QString& name,
                                     const QString& path, const QString& ghnsStatus);

      QTreeWidget* m_treeWidget{nullptr};
      QString m_backend;
};

#endif // QTHELPCONFIG_H
