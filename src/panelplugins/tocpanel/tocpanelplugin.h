/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _FILEBROWSERPANELPLUGIN_H
#define _FILEBROWSERPANELPLUGIN_H

#include <QHash>
#include <QList>
#include <QPoint>
#include <QPointer>
#include <QSet>
#include <QStandardItemModel>
#include <QString>
#include <QVariantList>
#include <QVector>
#include "panelplugin.h"

class QWidget;
class QEvent;
class QLineEdit;
class QLabel;
class QModelIndex;
class QMenu;
class QStandardItem;
class QTreeView;

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
    void requestNavigateToTocNode(QString nodeId);
    void requestRenameHierarchyEntry(QString hierarchyId, QString newName);
    void requestChangeHierarchyLevel(QString hierarchyId, int levelDelta);
    void requestDeleteHierarchyEntry(QString hierarchyId, bool deleteContents);
    void requestRenameCommandEntry(QString commandId, QString newTitle);
    void requestDeleteCommandEntry(QString commandId);
    void requestRenamePlot(QString commandId, QString resultId, QString newTitle);
    void requestDeletePlot(QString commandId, QString resultId);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  private Q_SLOTS:
    void handleClicked(const QModelIndex&);
    void handleDoubleClicked(const QModelIndex&);
    void handleTocNodeChanges(const QVariantList& nodes);
    void handleCurrentTocNodeChanged(const QString& nodeId);
    void handleExpanded(const QModelIndex& index);
    void handleCollapsed(const QModelIndex& index);
    void handleContextMenuRequested(const QPoint& position);
    void handleReadOnlyChanged(bool readOnly);

private:
    enum ItemRole
    {
        HierarchyIdRole = Qt::UserRole + 1,
        NodeIdRole,
        ParentNodeIdRole,
        NodeTypeRole,
        DepthRole,
        NameRole,
        DisplayTextRole,
        HierarchyTextRole,
        EntryIdRole,
        ResultIdRole,
        CustomTitleRole,
        ResultIndexRole,
        EditableRole,
        NavigableRole,
        CanPromoteRole,
        CanDemoteRole
    };

    struct TocNode
    {
        QString id;
        QString parentId;
        QString type;
        QString title;
        QString displayText;
        QString hierarchyText;
        QString hierarchyId;
        QString entryId;
        QString resultId;
        QString customTitle;
        int resultIndex{-1};
        int depth{0};
        int parentIndex{-1};
        bool editable{false};
        bool navigable{false};
        bool canPromote{false};
        bool canDemote{false};
    };

    void constructMainWidget();
    void rebuildModel();
    void applyTocNodeChanges(const QVariantList& nodes);
    void updateCurrentNodeSelection();
    void restoreExpansionState();
    void expandIndexParents(const QModelIndex& index);
    bool shouldDisplayNode(int index) const;
    int findVisibleAncestorIndex(int index) const;
    void clearNodes();
    void addVisibleItemsMenu(QMenu* menu);
    void resetVisibilityToDefaults();
    void cleanupStateAfterNodeChange();
    void beginRename(const QModelIndex& index);
    void handleEditorCommit(QWidget* editor);
    void handleEditorClosed();
    void finishEditorSession();
    void cancelEditorSession();
    void updateSearchVisibility();
    void saveCurrentExpansionState();
    void showContextMenuForIndex(const QModelIndex& index);
    void deleteItemAtIndex(const QModelIndex& index);

private:
    QPointer<QWidget> m_containerWidget;
    QPointer<QLineEdit> m_searchEdit;
    QPointer<QLabel> m_emptyLabel;
    QPointer<QTreeView> m_mainWidget;
    QStandardItemModel m_model;

    QVector<TocNode> m_nodes;
    QHash<QString, int> m_nodeIndexById;

    QHash<QString, QStandardItem*> m_itemsByNodeId;

    bool m_showChapters{true};
    bool m_showSections{true};
    bool m_showCommandEntries{false};
    bool m_showPlots{false};
    bool m_readOnly{false};

    QSet<QString> m_expandedNodeIds;
    QSet<QString> m_searchVisibleNodeIds;

    QString m_currentNodeId;
    QString m_searchText;
    bool m_updatingModel{false};
    bool m_modelFilteredBySearch{false};
    bool m_expansionStateInitialized{false};
    bool m_editorActive{false};
    bool m_hasPendingNodeSnapshot{false};
    bool m_hasPendingRename{false};
    QString m_pendingRenameNodeType;
    QString m_editingNodeId;
    QString m_pendingRenameHierarchyId;
    QString m_pendingRenameCommandId;
    QString m_pendingRenameResultId;
    QString m_pendingRenameTitle;
    QVariantList m_pendingNodeSnapshot;
};

#endif /* _FILEBROWSERPANELPLUGIN_H */
