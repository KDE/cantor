/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#include "tocpanelplugin.h"

#include <KPluginFactory>

#include <QAbstractItemDelegate>
#include <QAction>
#include <QDebug>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QModelIndex>
#include <QScopedValueRollback>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

#include "settings.h"

namespace
{
const QLatin1String TocStateVersion("toc-state-v2");
const QLatin1String TocNodeTypeChapter("chapter");
const QLatin1String TocNodeTypeSection("section");
const QLatin1String TocNodeTypeCommand("command");
const QLatin1String TocNodeTypePlot("plot");

class HierarchyNameDelegate : public QStyledItemDelegate
{
public:
    explicit HierarchyNameDelegate(int nameRole, int nodeIdRole, int hierarchyIdRole, int nodeTypeRole, int customTitleRole, int entryIdRole, int resultIdRole, QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
        , m_nameRole(nameRole)
        , m_nodeIdRole(nodeIdRole)
        , m_hierarchyIdRole(hierarchyIdRole)
        , m_nodeTypeRole(nodeTypeRole)
        , m_customTitleRole(customTitleRole)
        , m_entryIdRole(entryIdRole)
        , m_resultIdRole(resultIdRole)
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);

        auto* editor = new QLineEdit(parent);
        editor->setClearButtonEnabled(true);
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        auto* lineEdit = qobject_cast<QLineEdit*>(editor);

        if (!lineEdit)
        {
            QStyledItemDelegate::setEditorData(editor, index);
            return;
        }

        const QString nodeType = index.data(m_nodeTypeRole).toString();
        const bool isPlotNode = nodeType == TocNodeTypePlot;
        const QString title = index.data(m_nameRole).toString();
        const QString customTitle = index.data(m_customTitleRole).toString();
        const QString editTitle = isPlotNode && !customTitle.isEmpty() ? customTitle : title;

        lineEdit->setText(isPlotNode ? editTitle : title);
        if (isPlotNode)
            lineEdit->setPlaceholderText(title);

        lineEdit->setProperty("tocNodeId", index.data(m_nodeIdRole));
        lineEdit->setProperty("tocNodeType", nodeType);
        lineEdit->setProperty("tocHierarchyId", index.data(m_hierarchyIdRole));
        lineEdit->setProperty("tocCommandId", index.data(m_entryIdRole));
        lineEdit->setProperty("tocResultId", index.data(m_resultIdRole));
        lineEdit->setProperty("tocOriginalTitle", isPlotNode ? editTitle : title);
        lineEdit->selectAll();
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
        Q_UNUSED(editor);
        Q_UNUSED(model);
        Q_UNUSED(index);
    }

private:
    int m_nameRole;
    int m_nodeIdRole;
    int m_hierarchyIdRole;
    int m_nodeTypeRole;
    int m_customTitleRole;
    int m_entryIdRole;
    int m_resultIdRole;
};
}

TableOfContentPanelPlugin::TableOfContentPanelPlugin(QObject* parent, const QList<QVariant>& args): Cantor::PanelPlugin(parent),
    m_mainWidget(nullptr)
{
    Q_UNUSED(args);
    resetVisibilityToDefaults();
}

TableOfContentPanelPlugin::~TableOfContentPanelPlugin()
{
    if (m_mainWidget)
    {
        m_mainWidget->deleteLater();
    }
}

QWidget* TableOfContentPanelPlugin::widget()
{
    if (!m_mainWidget)
        constructMainWidget();

    return m_mainWidget;
}

void TableOfContentPanelPlugin::connectToShell(QObject* cantorShell)
{
    connect(cantorShell, SIGNAL(tocNodesChanged(QVariantList)), this, SLOT(handleTocNodeChanges(QVariantList)));
    connect(this, SIGNAL(requestNavigateToTocNode(QString)), cantorShell, SIGNAL(requestNavigateToTocNode(QString)));
    connect(this, SIGNAL(requestRenameHierarchyEntry(QString,QString)), cantorShell, SIGNAL(requestRenameHierarchyEntry(QString,QString)));
    connect(cantorShell, SIGNAL(currentTocNodeChanged(QString)), this, SLOT(handleCurrentTocNodeChanged(QString)));
    connect(this, SIGNAL(requestChangeHierarchyLevel(QString,int)), cantorShell, SIGNAL(requestChangeHierarchyLevel(QString,int)));
    connect(this, SIGNAL(requestDeleteHierarchyEntry(QString,bool)), cantorShell, SIGNAL(requestDeleteHierarchyEntry(QString,bool)));
    connect(this, SIGNAL(requestRenamePlot(QString,QString,QString)), cantorShell, SIGNAL(requestRenamePlot(QString,QString,QString)));
    connect(this, SIGNAL(requestDeletePlot(QString,QString)), cantorShell, SIGNAL(requestDeletePlot(QString,QString)));
    connect(cantorShell, SIGNAL(tocReadOnlyChanged(bool)), this, SLOT(handleReadOnlyChanged(bool)));
}

bool TableOfContentPanelPlugin::showOnStartup()
{
    return false;
}

void TableOfContentPanelPlugin::handleClicked(const QModelIndex& index)
{
    if (m_editorActive || !index.isValid())
        return;

    const QString nodeId = index.data(NodeIdRole).toString();
    const bool navigable = index.data(NavigableRole).toBool();

    if (nodeId.isEmpty() || !navigable)
        return;

    Q_EMIT requestNavigateToTocNode(nodeId);
}

void TableOfContentPanelPlugin::handleDoubleClicked(const QModelIndex& index)
{
    beginRename(index);
}

void TableOfContentPanelPlugin::constructMainWidget()
{
    auto* view = new QTreeView;

    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setHeaderHidden(true);
    view->setUniformRowHeights(true);
    view->setItemsExpandable(true);
    view->setExpandsOnDoubleClick(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->installEventFilter(this);
    view->viewport()->installEventFilter(this);

    auto* delegate = new HierarchyNameDelegate(NameRole, NodeIdRole, HierarchyIdRole, NodeTypeRole, CustomTitleRole, EntryIdRole, ResultIdRole, view);

    view->setItemDelegate(delegate);
    view->setModel(&m_model);

    connect(view, &QTreeView::clicked, this, &TableOfContentPanelPlugin::handleClicked);
    connect(view, &QTreeView::doubleClicked, this, &TableOfContentPanelPlugin::handleDoubleClicked);
    connect(view, &QTreeView::customContextMenuRequested, this, &TableOfContentPanelPlugin::handleContextMenuRequested);
    connect(view, &QTreeView::expanded, this, &TableOfContentPanelPlugin::handleExpanded);
    connect(view, &QTreeView::collapsed, this, &TableOfContentPanelPlugin::handleCollapsed);
    connect(delegate, &QAbstractItemDelegate::commitData, this, &TableOfContentPanelPlugin::handleEditorCommit);
    connect(delegate, &QAbstractItemDelegate::closeEditor, this, &TableOfContentPanelPlugin::handleEditorClosed);

    m_mainWidget = view;
    rebuildModel();
}

bool TableOfContentPanelPlugin::eventFilter(QObject* watched, QEvent* event)
{
    if (m_mainWidget && (watched == m_mainWidget || watched == m_mainWidget->viewport()) && event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_F2 && !m_readOnly)
        {
            beginRename(m_mainWidget->currentIndex());
            return true;
        }
    }

    return Cantor::PanelPlugin::eventFilter(watched, event);
}

void TableOfContentPanelPlugin::rebuildModel()
{
    QScopedValueRollback<bool> guard(m_updatingModel, true);

    m_model.clear();
    m_itemsByNodeId.clear();

    QVector<QStandardItem*> visibleItems(m_nodes.size(), nullptr);

    for (int i = 0; i < m_nodes.size(); ++i)
    {
        if (!shouldDisplayNode(i))
            continue;

        const TocNode& node = m_nodes.at(i);
        const int parentIndex = findVisibleAncestorIndex(node.parentIndex);
        QStandardItem* parentItem = parentIndex >= 0 ? visibleItems.at(parentIndex) : m_model.invisibleRootItem();

        auto* item = new QStandardItem(node.displayText);
        item->setEditable(node.editable && !m_readOnly);

        item->setData(node.id, NodeIdRole);
        item->setData(node.parentId, ParentNodeIdRole);
        item->setData(node.type, NodeTypeRole);
        item->setData(node.depth, DepthRole);
        item->setData(node.title, NameRole);
        item->setData(node.displayText, DisplayTextRole);
        item->setData(node.hierarchyText, HierarchyTextRole);
        item->setData(node.hierarchyId, HierarchyIdRole);
        item->setData(node.entryId, EntryIdRole);
        item->setData(node.resultId, ResultIdRole);
        item->setData(node.customTitle, CustomTitleRole);
        item->setData(node.resultIndex, ResultIndexRole);
        item->setData(node.editable, EditableRole);
        item->setData(node.navigable, NavigableRole);

        parentItem->appendRow(item);
        visibleItems[i] = item;

        m_itemsByNodeId.insert(node.id, item);
    }

    restoreExpansionState();
    updateCurrentNodeSelection();
}

void TableOfContentPanelPlugin::restoreExpansionState()
{
    if (!m_mainWidget)
        return;

    if (!m_expansionStateInitialized)
    {
        m_mainWidget->expandAll();

        for (auto it = m_itemsByNodeId.cbegin(); it != m_itemsByNodeId.cend(); ++it)
        {
            if (it.value() && it.value()->hasChildren())
                m_expandedNodeIds.insert(it.key());
        }

        m_expansionStateInitialized = true;
        return;
    }

    for (auto it = m_itemsByNodeId.cbegin(); it != m_itemsByNodeId.cend(); ++it)
    {
        if (!it.value())
            continue;

        const QModelIndex index = it.value()->index();

        if (m_expandedNodeIds.contains(it.key()))
            m_mainWidget->expand(index);
        else
            m_mainWidget->collapse(index);
    }
}

void TableOfContentPanelPlugin::updateCurrentNodeSelection()
{
    if (!m_mainWidget || !m_mainWidget->selectionModel())
        return;

    auto* selectionModel = m_mainWidget->selectionModel();

    auto* item = m_itemsByNodeId.value(m_currentNodeId, nullptr);
    if (!item)
    {
        const int sourceIndex = m_nodeIndexById.value(m_currentNodeId, -1);
        if (sourceIndex >= 0)
        {
            const int visibleAncestorIndex = findVisibleAncestorIndex(sourceIndex);
            if (visibleAncestorIndex >= 0)
                item = m_itemsByNodeId.value(m_nodes.at(visibleAncestorIndex).id, nullptr);
        }
    }

    if (!item)
    {
        selectionModel->clearSelection();
        selectionModel->setCurrentIndex(QModelIndex{}, QItemSelectionModel::NoUpdate);
        return;
    }

    const QModelIndex index = item->index();

    expandIndexParents(index);

    selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    m_mainWidget->scrollTo(index, QAbstractItemView::EnsureVisible);
}

void TableOfContentPanelPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    cancelEditorSession();
    clearNodes();

    resetVisibilityToDefaults();
    m_expandedNodeIds.clear();
    m_currentNodeId.clear();
    m_expansionStateInitialized = false;

    bool restoredExpansionState = false;

    if (state.inners.isEmpty())
    {
        rebuildModel();
        return;
    }

    if (state.inners.at(0).toString() == TocStateVersion)
    {
        if (state.inners.size() > 1)
            m_showChapters = state.inners.at(1).toBool();
        if (state.inners.size() > 2)
            m_showSections = state.inners.at(2).toBool();
        if (state.inners.size() > 3)
            m_showCommandEntries = state.inners.at(3).toBool();
        if (state.inners.size() > 4)
            m_showPlots = state.inners.at(4).toBool();

        const QStringList expandedIds = state.inners.size() > 5 ? state.inners.at(5).toStringList() : QStringList();
        for (const QString& nodeId : expandedIds)
            m_expandedNodeIds.insert(nodeId);

        if (state.inners.size() > 6)
            m_currentNodeId = state.inners.at(6).toString();

        restoredExpansionState = true;
    }
    else if (state.inners.size() >= 10)
    {
        const QStringList expandedIds = state.inners.at(4).toStringList();
        for (const QString& nodeId : expandedIds)
            m_expandedNodeIds.insert(nodeId);

        m_currentNodeId = state.inners.at(5).toString();
        m_showChapters = state.inners.at(6).toBool();
        m_showSections = state.inners.at(7).toBool();
        m_showCommandEntries = state.inners.at(8).toBool();
        m_showPlots = state.inners.at(9).toBool();
        restoredExpansionState = true;
    }
    else if (state.inners.size() >= 6)
    {
        const QStringList expandedIds = state.inners.at(4).toStringList();
        for (const QString& nodeId : expandedIds)
            m_expandedNodeIds.insert(nodeId);

        m_currentNodeId = state.inners.at(5).toString();
        restoredExpansionState = true;
    }

    m_expansionStateInitialized = restoredExpansionState;

    rebuildModel();
}

void TableOfContentPanelPlugin::expandIndexParents(const QModelIndex& index)
{
    if (!m_mainWidget)
        return;

    QList<QModelIndex> parents;

    for (QModelIndex parent = index.parent(); parent.isValid(); parent = parent.parent())
        parents.prepend(parent);

    for (const QModelIndex& parent : parents)
        m_mainWidget->expand(parent);
}

Cantor::PanelPlugin::State TableOfContentPanelPlugin::saveState()
{
    Cantor::PanelPlugin::State state;

    state.inners.append(QString(TocStateVersion));
    state.inners.append(m_showChapters);
    state.inners.append(m_showSections);
    state.inners.append(m_showCommandEntries);
    state.inners.append(m_showPlots);
    state.inners.append(QStringList(m_expandedNodeIds.values()));
    state.inners.append(m_currentNodeId);

    return state;
}

void TableOfContentPanelPlugin::handleTocNodeChanges(const QVariantList& nodes)
{
    if (m_editorActive)
    {
        m_pendingNodeSnapshot = nodes;
        m_hasPendingNodeSnapshot = true;
        return;
    }

    applyTocNodeChanges(nodes);
}

void TableOfContentPanelPlugin::applyTocNodeChanges(const QVariantList& nodes)
{
    clearNodes();
    m_nodes.reserve(nodes.size());

    QSet<QString> seenNodeIds;

    const auto nodeTypeFromValue = [](const QVariant& value) -> QString
    {
        if (value.typeId() == QMetaType::QString)
            return value.toString();

        bool ok = false;
        const int legacyType = value.toInt(&ok);
        if (!ok)
            return QString();

        switch (legacyType)
        {
            case 0:
                return TocNodeTypeChapter;
            case 1:
                return TocNodeTypeSection;
            case 2:
                return TocNodeTypeCommand;
            case 3:
                return TocNodeTypePlot;
            default:
                return QString();
        }
    };

    for (const QVariant& value : nodes)
    {
        const QVariantMap node = value.toMap();

        TocNode tocNode;
        tocNode.id = node.value(QStringLiteral("id")).toString();
        tocNode.parentId = node.value(QStringLiteral("parentId")).toString();
        tocNode.type = nodeTypeFromValue(node.value(QStringLiteral("type")));
        tocNode.title = node.value(QStringLiteral("title")).toString();
        tocNode.displayText = node.value(QStringLiteral("displayText")).toString();
        tocNode.hierarchyText = node.value(QStringLiteral("hierarchyText")).toString();
        tocNode.hierarchyId = node.value(QStringLiteral("hierarchyId")).toString();
        tocNode.entryId = node.value(QStringLiteral("entryId")).toString();
        tocNode.resultId = node.value(QStringLiteral("resultId")).toString();
        tocNode.customTitle = node.value(QStringLiteral("customTitle")).toString();
        tocNode.resultIndex = node.value(QStringLiteral("resultIndex"), -1).toInt();
        tocNode.depth = node.value(QStringLiteral("depth"), 0).toInt();
        tocNode.editable = node.value(QStringLiteral("editable"), false).toBool();
        tocNode.navigable = node.value(QStringLiteral("navigable"), false).toBool();

        if (tocNode.displayText.isEmpty())
            tocNode.displayText = tocNode.title;

        if (tocNode.id.isEmpty() || tocNode.type.isEmpty() || seenNodeIds.contains(tocNode.id))
        {
            qWarning() << "Ignoring invalid TOC node" << node;
            continue;
        }

        seenNodeIds.insert(tocNode.id);
        m_nodeIndexById.insert(tocNode.id, m_nodes.size());
        m_nodes.append(tocNode);
    }

    for (int i = 0; i < m_nodes.size(); ++i)
    {
        const QString& parentId = m_nodes.at(i).parentId;
        m_nodes[i].parentIndex = parentId.isEmpty() ? -1 : m_nodeIndexById.value(parentId, -1);
    }

    cleanupStateAfterNodeChange();
    rebuildModel();
}

void TableOfContentPanelPlugin::beginRename(const QModelIndex& index)
{
    if (!m_mainWidget || !index.isValid() || m_editorActive || m_readOnly)
        return;

    const QString nodeId = index.data(NodeIdRole).toString();
    const QString hierarchyId = index.data(HierarchyIdRole).toString();
    const QString nodeType = index.data(NodeTypeRole).toString();
    const bool isPlotNode = nodeType == TocNodeTypePlot;

    if (nodeId.isEmpty() || !index.data(EditableRole).toBool())
        return;

    if (!isPlotNode && hierarchyId.isEmpty())
        return;

    if (isPlotNode && (index.data(EntryIdRole).toString().isEmpty() || index.data(ResultIdRole).toString().isEmpty()))
        return;

    m_editorActive = true;
    m_editingNodeId = nodeId;
    m_hasPendingRename = false;
    m_pendingRenameNodeType.clear();
    m_pendingRenameHierarchyId.clear();
    m_pendingRenameCommandId.clear();
    m_pendingRenameResultId.clear();
    m_pendingRenameTitle.clear();

    m_mainWidget->setCurrentIndex(index);
    m_mainWidget->edit(index);

    QTimer::singleShot(0, this, [this, nodeId]()
    {
        if (!m_editorActive || m_editingNodeId != nodeId || !m_mainWidget)
            return;

        const auto editors = m_mainWidget->findChildren<QLineEdit*>();
        for (auto* editor : editors)
        {
            if (editor && editor->property("tocNodeId").toString() == nodeId)
                return;
        }

        finishEditorSession();
    });
}

void TableOfContentPanelPlugin::handleEditorCommit(QWidget* editor)
{
    if (!m_editorActive || !editor)
        return;

    auto* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit)
        return;

    const QString nodeId = lineEdit->property("tocNodeId").toString();
    const QString nodeType = lineEdit->property("tocNodeType").toString();
    const QString hierarchyId = lineEdit->property("tocHierarchyId").toString();
    const QString commandId = lineEdit->property("tocCommandId").toString();
    const QString resultId = lineEdit->property("tocResultId").toString();

    if (nodeId != m_editingNodeId)
        return;

    QString newName = lineEdit->text();
    newName.replace(QLatin1Char('\r'), QLatin1Char(' '));
    newName.replace(QLatin1Char('\n'), QLatin1Char(' '));
    newName = newName.trimmed();

    QString oldName = lineEdit->property("tocOriginalTitle").toString();
    const int position = m_nodeIndexById.value(nodeId, -1);
    if (position >= 0 && position < m_nodes.size())
    {
        const TocNode& node = m_nodes.at(position);
        oldName = nodeType == TocNodeTypePlot && !node.customTitle.isEmpty() ? node.customTitle : node.title;
    }

    if (newName == oldName)
        return;

    if (nodeType == TocNodeTypePlot)
    {
        if (commandId.isEmpty() || resultId.isEmpty())
            return;

        m_pendingRenameCommandId = commandId;
        m_pendingRenameResultId = resultId;
    }
    else
    {
        if (hierarchyId.isEmpty())
            return;

        m_pendingRenameHierarchyId = hierarchyId;
    }

    m_pendingRenameNodeType = nodeType;
    m_pendingRenameTitle = newName;
    m_hasPendingRename = true;
}

void TableOfContentPanelPlugin::handleEditorClosed()
{
    QTimer::singleShot(0, this, &TableOfContentPanelPlugin::finishEditorSession);
}

void TableOfContentPanelPlugin::finishEditorSession()
{
    const bool hadEditor = m_editorActive;
    m_editorActive = false;
    m_editingNodeId.clear();

    if (!hadEditor && !m_hasPendingNodeSnapshot && !m_hasPendingRename)
        return;

    if (m_hasPendingNodeSnapshot)
    {
        const QVariantList pendingNodes = m_pendingNodeSnapshot;
        m_pendingNodeSnapshot.clear();
        m_hasPendingNodeSnapshot = false;
        applyTocNodeChanges(pendingNodes);
    }

    if (m_hasPendingRename)
    {
        const QString nodeType = m_pendingRenameNodeType;
        const QString hierarchyId = m_pendingRenameHierarchyId;
        const QString commandId = m_pendingRenameCommandId;
        const QString resultId = m_pendingRenameResultId;
        const QString title = m_pendingRenameTitle;

        m_pendingRenameNodeType.clear();
        m_pendingRenameHierarchyId.clear();
        m_pendingRenameCommandId.clear();
        m_pendingRenameResultId.clear();
        m_pendingRenameTitle.clear();
        m_hasPendingRename = false;

        if (nodeType == TocNodeTypePlot)
            Q_EMIT requestRenamePlot(commandId, resultId, title);
        else
            Q_EMIT requestRenameHierarchyEntry(hierarchyId, title);
    }
}

void TableOfContentPanelPlugin::cancelEditorSession()
{
    m_editorActive = false;
    m_editingNodeId.clear();
    m_hasPendingNodeSnapshot = false;
    m_hasPendingRename = false;
    m_pendingRenameNodeType.clear();
    m_pendingRenameHierarchyId.clear();
    m_pendingRenameCommandId.clear();
    m_pendingRenameResultId.clear();
    m_pendingRenameTitle.clear();
    m_pendingNodeSnapshot.clear();
}

bool TableOfContentPanelPlugin::shouldDisplayNode(int index) const
{
    if (index < 0 || index >= m_nodes.size())
        return false;

    const QString& type = m_nodes.at(index).type;

    if (type == TocNodeTypeChapter)
        return m_showChapters;
    if (type == TocNodeTypeSection)
        return m_showSections;
    if (type == TocNodeTypeCommand)
        return m_showCommandEntries;
    if (type == TocNodeTypePlot)
        return m_showPlots;

    return true;
}

int TableOfContentPanelPlugin::findVisibleAncestorIndex(int index) const
{
    while (index >= 0)
    {
        if (shouldDisplayNode(index))
            return index;

        index = m_nodes.value(index).parentIndex;
    }
    return -1;
}

void TableOfContentPanelPlugin::clearNodes()
{
    m_nodes.clear();
    m_nodeIndexById.clear();
    m_itemsByNodeId.clear();
}

void TableOfContentPanelPlugin::resetVisibilityToDefaults()
{
    m_showChapters = Settings::showTocChaptersDefault();
    m_showSections = Settings::showTocSectionsDefault();
    m_showCommandEntries = Settings::showTocCommandEntriesDefault();
    m_showPlots = Settings::showTocPlotsDefault();
}

void TableOfContentPanelPlugin::cleanupStateAfterNodeChange()
{
    for (auto it = m_expandedNodeIds.begin(); it != m_expandedNodeIds.end();)
    {
        if (!m_nodeIndexById.contains(*it))
            it = m_expandedNodeIds.erase(it);
        else
            ++it;
    }

    if (!m_currentNodeId.isEmpty() && !m_nodeIndexById.contains(m_currentNodeId))
        m_currentNodeId.clear();
}

void TableOfContentPanelPlugin::addVisibleItemsMenu(QMenu* menu)
{
    if (!menu)
        return;

    QMenu* visibleItemsMenu = menu->addMenu(i18n("Visible Items"));

    QAction* showChaptersAction = visibleItemsMenu->addAction(i18n("Show Chapters"));
    showChaptersAction->setCheckable(true);
    showChaptersAction->setChecked(m_showChapters);
    connect(showChaptersAction, &QAction::triggered, this, [this](bool checked)
    {
        m_showChapters = checked;
        rebuildModel();
    });

    QAction* showSectionsAction = visibleItemsMenu->addAction(i18n("Show Sections"));
    showSectionsAction->setCheckable(true);
    showSectionsAction->setChecked(m_showSections);
    connect(showSectionsAction, &QAction::triggered, this, [this](bool checked)
    {
        m_showSections = checked;
        rebuildModel();
    });

    QAction* showCommandsAction = visibleItemsMenu->addAction(i18n("Show Command Entries"));
    showCommandsAction->setCheckable(true);
    showCommandsAction->setChecked(m_showCommandEntries);
    connect(showCommandsAction, &QAction::triggered, this, [this](bool checked)
    {
        m_showCommandEntries = checked;
        rebuildModel();
    });

    QAction* showPlotsAction = visibleItemsMenu->addAction(i18n("Show Plots"));
    showPlotsAction->setCheckable(true);
    showPlotsAction->setChecked(m_showPlots);
    connect(showPlotsAction, &QAction::triggered, this, [this](bool checked)
    {
        m_showPlots = checked;
        rebuildModel();
    });

    visibleItemsMenu->addSeparator();

    QAction* resetAction = visibleItemsMenu->addAction(QIcon::fromTheme(QStringLiteral("edit-undo")), i18n("Reset to Defaults"));
    connect(resetAction, &QAction::triggered, this, [this]()
    {
        resetVisibilityToDefaults();
        rebuildModel();
    });
}

void TableOfContentPanelPlugin::handleCurrentTocNodeChanged(const QString& nodeId)
{
    m_currentNodeId = nodeId;
    updateCurrentNodeSelection();
}

void TableOfContentPanelPlugin::handleReadOnlyChanged(bool readOnly)
{
    if (m_readOnly == readOnly)
        return;

    m_readOnly = readOnly;

    if (m_editorActive)
        finishEditorSession();

    rebuildModel();
}

void TableOfContentPanelPlugin::handleExpanded(const QModelIndex& index)
{
    const QString nodeId = index.data(NodeIdRole).toString();

    if (!nodeId.isEmpty())
        m_expandedNodeIds.insert(nodeId);
}

void TableOfContentPanelPlugin::handleCollapsed(const QModelIndex& index)
{
    const QString nodeId = index.data(NodeIdRole).toString();

    if (!nodeId.isEmpty())
        m_expandedNodeIds.remove(nodeId);
}

void TableOfContentPanelPlugin::handleContextMenuRequested(const QPoint& position)
{
    if (!m_mainWidget)
        return;

    if (m_editorActive)
        return;

    const QModelIndex index = m_mainWidget->indexAt(position);

    QMenu menu(m_mainWidget);

    if (index.isValid())
    {
        const QString nodeId = index.data(NodeIdRole).toString();
        const QString hierarchyId = index.data(HierarchyIdRole).toString();
        const QString nodeType = index.data(NodeTypeRole).toString();

        auto* item = m_model.itemFromIndex(index);

        const int depth = index.data(DepthRole).toInt();

        if (!hierarchyId.isEmpty())
        {
            QAction* goToHeadingAction = menu.addAction(i18n("Go to Heading"));
            goToHeadingAction->setEnabled(!nodeId.isEmpty() && index.data(NavigableRole).toBool());
            connect(goToHeadingAction, &QAction::triggered, this, [this, nodeId]()
            {
                if (!nodeId.isEmpty())
                    Q_EMIT requestNavigateToTocNode(nodeId);
            });

            menu.addSeparator();

            QAction* renameAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename"));
            renameAction->setEnabled(!m_readOnly);

            connect(renameAction, &QAction::triggered, this, [this, nodeId]()
            {
                if (auto* item = m_itemsByNodeId.value(nodeId, nullptr))
                    beginRename(item->index());
            });

            menu.addSeparator();

            QAction* promoteAction = menu.addAction(QIcon::fromTheme(QStringLiteral("format-indent-less")), i18n("Promote Heading"));
            promoteAction->setEnabled(!m_readOnly && depth > 0);
            connect(promoteAction, &QAction::triggered, this, [this, hierarchyId]()
            {
                Q_EMIT requestChangeHierarchyLevel(hierarchyId, -1);
            });

            QAction* demoteAction = menu.addAction(QIcon::fromTheme(QStringLiteral("format-indent-more")), i18n("Demote Heading"));
            demoteAction->setEnabled(!m_readOnly && depth < 5);
            connect(demoteAction, &QAction::triggered, this, [this, hierarchyId]()
            {
                Q_EMIT requestChangeHierarchyLevel(hierarchyId, 1);
            });

            menu.addSeparator();

            QAction* deleteHeadingAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete Heading Only"));
            deleteHeadingAction->setEnabled(!m_readOnly);
            connect(deleteHeadingAction, &QAction::triggered, this, [this, hierarchyId]()
            {
                Q_EMIT requestDeleteHierarchyEntry(hierarchyId, false);
            });

            QAction* deleteSectionAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete Section and Contents"));
            deleteSectionAction->setEnabled(!m_readOnly);
            connect(deleteSectionAction, &QAction::triggered, this, [this, hierarchyId]()
            {
                Q_EMIT requestDeleteHierarchyEntry(hierarchyId, true);
            });
        }
        else if (nodeType == TocNodeTypeCommand)
        {
            QAction* goToCommandAction = menu.addAction(i18n("Go to Command"));
            goToCommandAction->setEnabled(!nodeId.isEmpty() && index.data(NavigableRole).toBool());
            connect(goToCommandAction, &QAction::triggered, this, [this, nodeId]()
            {
                if (!nodeId.isEmpty())
                    Q_EMIT requestNavigateToTocNode(nodeId);
            });
        }
        else if (nodeType == TocNodeTypePlot)
        {
            const QString commandId = index.data(EntryIdRole).toString();
            const QString resultId = index.data(ResultIdRole).toString();
            const QString displayText = index.data(DisplayTextRole).toString();
            QAction* goToPlotAction = menu.addAction(i18n("Go to Plot"));
            goToPlotAction->setEnabled(!nodeId.isEmpty() && index.data(NavigableRole).toBool());
            connect(goToPlotAction, &QAction::triggered, this, [this, nodeId]()
            {
                if (!nodeId.isEmpty())
                    Q_EMIT requestNavigateToTocNode(nodeId);
            });

            menu.addSeparator();

            QAction* renameAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename Plot"));
            renameAction->setEnabled(!m_readOnly && !nodeId.isEmpty() && !commandId.isEmpty() && !resultId.isEmpty());
            connect(renameAction, &QAction::triggered, this, [this, nodeId]()
            {
                if (auto* item = m_itemsByNodeId.value(nodeId, nullptr))
                    beginRename(item->index());
            });

            QAction* deleteAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete Plot"));
            deleteAction->setEnabled(!m_readOnly && !commandId.isEmpty() && !resultId.isEmpty());
            connect(deleteAction, &QAction::triggered, this, [this, commandId, resultId, displayText]()
            {
                const QString plotTitle = displayText.isEmpty() ? i18n("Plot") : displayText;
                const auto result = KMessageBox::warningTwoActions(
                    m_mainWidget,
                    i18n("Do you really want to delete \"%1\"? This action cannot be undone.", plotTitle),
                    i18n("Delete Plot"),
                    KStandardGuiItem::remove(),
                    KStandardGuiItem::cancel());

                if (result == KMessageBox::PrimaryAction)
                    Q_EMIT requestDeletePlot(commandId, resultId);
            });
        }

        if (item && item->hasChildren())
        {
            menu.addSeparator();

            if (m_mainWidget->isExpanded(index))
            {
                QAction* collapseAction = menu.addAction(QIcon::fromTheme(QStringLiteral("arrow-up-double")), i18n("Collapse Section"));
                connect(collapseAction, &QAction::triggered, this, [this, nodeId]()
                {
                    if (m_mainWidget)
                    {
                        if (auto* item = m_itemsByNodeId.value(nodeId, nullptr))
                            m_mainWidget->collapse(item->index());
                    }
                });
            }
            else
            {
                QAction* expandAction = menu.addAction(QIcon::fromTheme(QStringLiteral("arrow-down-double")), i18n("Expand Section"));
                connect(expandAction, &QAction::triggered, this, [this, nodeId]()
                {
                    if (m_mainWidget)
                    {
                        if (auto* item = m_itemsByNodeId.value(nodeId, nullptr))
                            m_mainWidget->expandRecursively(item->index());
                    }
                });
            }
        }

        menu.addSeparator();
    }

    addVisibleItemsMenu(&menu);
    menu.addSeparator();

    QAction* expandAllAction = menu.addAction(QIcon::fromTheme(QStringLiteral("view-list-tree")), i18n("Expand All"));
    connect(expandAllAction, &QAction::triggered, this, [this]()
    {
        if (m_mainWidget)
            m_mainWidget->expandAll();
    });

    QAction* collapseAllAction = menu.addAction(QIcon::fromTheme(QStringLiteral("view-list-details")), i18n("Collapse All"));
    connect(collapseAllAction, &QAction::triggered, this, [this]()
    {
        if (m_mainWidget)
            m_mainWidget->collapseAll();
    });

    menu.exec(m_mainWidget->viewport()->mapToGlobal(position));
}

K_PLUGIN_FACTORY_WITH_JSON(tocpanelplugin, "tocpanelplugin.json", registerPlugin<TableOfContentPanelPlugin>();)
#include "tocpanelplugin.moc"
