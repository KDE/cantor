/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _VARIABLEMANAGERWIDGET_H
#define _VARIABLEMANAGERWIDGET_H

#include <QWidget>

namespace Cantor{
    class Session;
}

class QAbstractItemModel;
class QLineEdit;
class QToolButton;
class QTreeView;

class VariableManagerWidget : public QWidget
{
  Q_OBJECT
public:
    VariableManagerWidget(Cantor::Session*, QWidget*);
    ~VariableManagerWidget() override = default;

    void setSession(Cantor::Session*);

public Q_SLOTS:
    void clearVariables();

    void save();
    void load();
    void newVariable();

Q_SIGNALS:
    void runCommand(const QString&);

private:
    Cantor::Session* m_session{nullptr};
    QAbstractItemModel* m_model{nullptr};
    QTreeView* m_treeView{nullptr};
    QToolButton* m_newBtn{nullptr};
    QToolButton* m_loadBtn{nullptr};
    QToolButton* m_saveBtn{nullptr};
    QToolButton* m_clearBtn{nullptr};
    QLineEdit* m_leFilter;
    QToolButton* m_bFilterOptions;
    QAction* m_caseSensitiveAction;
    QAction* m_matchCompleteWordAction;

private Q_SLOTS:
    void filterTextChanged(const QString&);
    void toggleFilterOptionsMenu(bool);
    void updateButtons();
};

#endif /* _VARIABLEMANAGERWIDGET_H */
