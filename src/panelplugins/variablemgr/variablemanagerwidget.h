/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _VARIABLEMANAGERWIDGET_H
#define _VARIABLEMANAGERWIDGET_H

#include <QWidget>

namespace Cantor{
class Session;
}

class QTreeView;
class QToolButton;
class QAbstractItemModel;

class VariableManagerWidget : public QWidget
{
  Q_OBJECT
  public:
    VariableManagerWidget( Cantor::Session* session,QWidget* parent );
    ~VariableManagerWidget() override = default;

    void setSession(Cantor::Session* session);

  public Q_SLOTS:
    void clearVariables();

    void save();
    void load();
    void newVariable();

  Q_SIGNALS:
    void runCommand(const QString& cmd);

  private:
    Cantor::Session* m_session;
    QAbstractItemModel* m_model;
    QTreeView* m_table;
    QToolButton* m_newBtn;
    QToolButton* m_loadBtn;
    QToolButton* m_saveBtn;
    QToolButton* m_clearBtn;
};

#endif /* _VARIABLEMANAGERWIDGET_H */
