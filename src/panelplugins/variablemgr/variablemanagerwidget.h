/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2021-2022 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _VARIABLEMANAGERWIDGET_H
#define _VARIABLEMANAGERWIDGET_H

#include "variablepreview.h"

#include <QHash>
#include <QPointer>
#include <QWidget>

namespace Cantor{
    class Session;
}

class QAbstractItemModel;
class QLineEdit;
class QToolButton;
class QTreeView;
class VariablePreviewWindow;

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
    QLineEdit* m_leFilter{nullptr};
    QToolButton* m_bFilterOptions{nullptr};
    QAction* m_caseSensitiveAction{nullptr};
    QAction* m_matchCompleteWordAction{nullptr};
    QAction* m_copyNameAction{nullptr};
    QAction* m_copyValueAction{nullptr};
    QAction* m_copyNameValueAction{nullptr};
    QAction* m_previewAction{nullptr};
    QToolButton* m_previewBtn{nullptr};
    QHash<QString, QPointer<VariablePreviewWindow>> m_previewWindows;

    void contextMenuEvent(QContextMenuEvent*) override;
    Cantor::VariablePreviewData::Reference selectedPreview() const;
    void openPreview(const Cantor::VariablePreviewData::Reference& reference);
    void closePreviews();

private Q_SLOTS:
    void filterTextChanged(const QString&);
    void toggleFilterOptionsMenu(bool);
    void updateButtons();
    void updatePreviewAction();
    void previewSelected();
    void markPreviewsStale();
    void resizeNameColumn();
    void copy(const QAction*) const;
};

#endif /* _VARIABLEMANAGERWIDGET_H */
