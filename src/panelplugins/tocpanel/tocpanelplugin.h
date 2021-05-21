/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _FILEBROWSERPANELPLUGIN_H
#define _FILEBROWSERPANELPLUGIN_H

#include <QStringListModel>

#include "panelplugin.h"

class QWidget;
class QModelIndex;
class QPushButton;
class QLineEdit;
class QComboBox;

class TableOfContentPanelPlugin : public Cantor::PanelPlugin
{
  Q_OBJECT
  public:
    TableOfContentPanelPlugin (QObject* parent, const QList<QVariant>& args);
    ~TableOfContentPanelPlugin() override;

    QWidget* widget() override;

    bool showOnStartup() override;

    void connectToShell(QObject * cantorShell) override;

    State saveState() override;

    void restoreState(const State& state) override;


  Q_SIGNALS:
    void requestScrollToHierarchyEntry(QString);

  private Q_SLOTS:
    void handleDoubleClicked(const QModelIndex&);
    void handleHierarchyChanges(QStringList names, QStringList searchStrings, QList<int> depths);
    void handleHierarhyEntryNameChange(QString name, QString searchString, int deapth);

  private:
    void constructMainWidget();

  private:
    QPointer<QWidget> m_mainWidget;
    QStringListModel m_model;
    QStringList m_hierarchyPositionStringList;
};

#endif /* _FILEBROWSERPANELPLUGIN_H */
