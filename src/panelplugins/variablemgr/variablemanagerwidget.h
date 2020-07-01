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
    Copyright (C) 2010 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _VARIABLEMANAGERWIDGET_H
#define _VARIABLEMANAGERWIDGET_H

#include <QWidget>

namespace Cantor{
class Session;
}

class QTreeView;
class QAbstractItemModel;

class VariableManagerWidget : public QWidget
{
  Q_OBJECT
  public:
    VariableManagerWidget(Cantor::Session* session, QWidget* parent);
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
};

#endif /* _VARIABLEMANAGERWIDGET_H */
