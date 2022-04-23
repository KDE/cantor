/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _FILEBROWSERPANELPLUGIN_H
#define _FILEBROWSERPANELPLUGIN_H

#include <QPointer>
#include <QVector>

#include "panelplugin.h"

class QTreeView;
class QWidget;
class QModelIndex;
class QFileSystemModel;
class QPushButton;
class QLineEdit;
class QComboBox;

class FileBrowserPanelPlugin : public Cantor::PanelPlugin
{
  Q_OBJECT
  public:
    FileBrowserPanelPlugin (QObject* parent, const QList<QVariant>& args);
    ~FileBrowserPanelPlugin() override;

    QWidget* widget() override;

    bool showOnStartup() override;

    void connectToShell(QObject * cantorShell) override;

    // No use restore files, because the FileBrowser Panel can be shared between session

  Q_SIGNALS:
    void requestOpenWorksheet(const QUrl&);

  private Q_SLOTS:
    void handleDoubleClicked(const QModelIndex&);
    void dirPreviousButtonHandle();
    void dirUpButtonHandle();
    void homeButtonHandle();
    void dirNextButtonHandle();
    void setNewRootPath();
    void handleFilterChanging(int index);

  private:
    void constructMainWidget();
    void moveFileBrowserRoot(const QString& path);
    void setRootPath(const QString& path);

  private:
    QPointer<QWidget> m_mainWidget;
    QPointer<QTreeView> m_treeview;
    QPointer<QLineEdit> m_pathEdit;
    QPointer<QComboBox> m_filterCombobox;
    QFileSystemModel* m_model = nullptr;
    QVector<QString> m_rootDirsHistory;
    int historyBackCount = 0;
};

#endif /* _FILEBROWSERPANELPLUGIN_H */
