#include "plotpanelplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <QAbstractItemView>
#include <QAction>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QSet>
#include <QSignalBlocker>
#include <QSize>
#include <QSizePolicy>
#include <QStackedLayout>
#include <QTimer>
#include <QToolBar>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include <utility>

namespace
{
const QLatin1String PlotNodeType("plot");
constexpr QSize ThumbnailSize(240, 180);
constexpr int AnimationRefreshInterval = 66;

QImage scaledPreview(QImage preview, Qt::TransformationMode transformationMode)
{
    if (preview.isNull())
        return preview;

    if (preview.width() > ThumbnailSize.width() || preview.height() > ThumbnailSize.height())
        preview = preview.scaled(ThumbnailSize, Qt::KeepAspectRatio, transformationMode);
    preview.setDevicePixelRatio(1.0);
    return preview;
}

QIcon previewIcon(const QImage& preview, Qt::TransformationMode transformationMode = Qt::SmoothTransformation)
{
    if (!preview.isNull())
        return QIcon(QPixmap::fromImage(scaledPreview(preview, transformationMode)));

    constexpr QSize missingIconSize(64, 64);
    QPixmap placeholder(ThumbnailSize);
    placeholder.fill(Qt::transparent);

    const QPixmap missingIcon = QIcon::fromTheme(QStringLiteral("image-missing")).pixmap(missingIconSize);
    QPainter painter(&placeholder);
    const QPoint missingIconPosition((ThumbnailSize.width() - missingIcon.width()) / 2, (ThumbnailSize.height() - missingIcon.height()) / 2);
    painter.drawPixmap(QRect(missingIconPosition, missingIcon.size()), missingIcon);
    return QIcon(placeholder);
}
}

PlotPanelPlugin::PlotPanelPlugin(QObject* parent, const QList<QVariant>& args) : Cantor::PanelPlugin(parent)
{
    Q_UNUSED(args);
}

PlotPanelPlugin::~PlotPanelPlugin()
{
    clearPlots();
    if (m_containerWidget)
        m_containerWidget->deleteLater();
}

QWidget* PlotPanelPlugin::widget()
{
    if (!m_containerWidget)
        constructWidget();

    return m_containerWidget;
}

bool PlotPanelPlugin::showOnStartup()
{
    return false;
}

void PlotPanelPlugin::connectToShell(QObject* cantorShell)
{
    connect(cantorShell, SIGNAL(tocNodesChanged(QVariantList)), this, SLOT(handleTocNodesChanged(QVariantList)));
    connect(cantorShell, SIGNAL(currentTocNodeChanged(QString)), this, SLOT(handleCurrentTocNodeChanged(QString)));
    connect(cantorShell, SIGNAL(plotAnimationFrameChanged(QString,QImage)), this, SLOT(handleAnimationFrameChanged(QString,QImage)));
    connect(cantorShell, SIGNAL(tocReadOnlyChanged(bool)), this, SLOT(handleReadOnlyChanged(bool)));
    connect(this, SIGNAL(requestNavigateToTocNode(QString)), cantorShell, SIGNAL(requestNavigateToTocNode(QString)));
    connect(this, SIGNAL(requestRenamePlot(QString,QString,QString)), cantorShell, SIGNAL(requestRenamePlot(QString,QString,QString)));
    connect(this, SIGNAL(requestDeletePlot(QString,QString)), cantorShell, SIGNAL(requestDeletePlot(QString,QString)));
    connect(this, SIGNAL(requestSavePlot(QString,QString)), cantorShell, SIGNAL(requestSavePlot(QString,QString)));
    connect(this, SIGNAL(requestSaveAllPlots()), cantorShell, SIGNAL(requestSaveAllPlots()));
    connect(this, SIGNAL(requestCopyPlot(QString,QString)), cantorShell, SIGNAL(requestCopyPlot(QString,QString)));
}

Cantor::PanelPlugin::State PlotPanelPlugin::saveState()
{
    State state = PanelPlugin::saveState();
    state.inners.append(m_currentNodeId);
    state.inners.append(m_animatePreviews);
    state.inners.append(m_showSourceLabels);
    return state;
}

void PlotPanelPlugin::restoreState(const State& state)
{
    PanelPlugin::restoreState(state);

    m_currentNodeId.clear();
    m_animatePreviews = true;
    m_showSourceLabels = false;
    if (!state.inners.isEmpty())
        m_currentNodeId = state.inners.at(0).toString();
    if (state.inners.size() > 1)
        m_animatePreviews = state.inners.at(1).toBool();
    if (state.inners.size() > 2)
        m_showSourceLabels = state.inners.at(2).toBool();

    if (m_animationAction)
        m_animationAction->setChecked(m_animatePreviews);
    if (m_sourceLabelsAction)
        m_sourceLabelsAction->setChecked(m_showSourceLabels);
    updateItemLabels();
}

void PlotPanelPlugin::constructWidget()
{
    m_containerWidget = new QWidget(parentWidget());
    m_containerWidget->setAccessibleName(i18n("Plots panel"));

    auto* outerLayout = new QVBoxLayout(m_containerWidget);
    outerLayout->setContentsMargins({});
    outerLayout->setSpacing(4);

    m_toolbar = new QToolBar(m_containerWidget);
    m_toolbar->setIconSize(QSize(16, 16));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    outerLayout->addWidget(m_toolbar);

    m_previousAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("go-previous")), i18n("Previous Plot"));
    m_nextAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("go-next")), i18n("Next Plot"));
    m_toolbar->addSeparator();
    m_sourceLabelsAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("view-list-tree")), i18n("Group Plots by Source Cell"));
    m_sourceLabelsAction->setCheckable(true);
    m_sourceLabelsAction->setChecked(m_showSourceLabels);
    m_animationAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("media-playback-start")), i18n("Animate Previews"));
    m_animationAction->setCheckable(true);
    m_animationAction->setChecked(m_animatePreviews);
    m_toolbar->addSeparator();
    m_copyAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy Plot"));
    m_saveAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("document-save")), i18n("Save Plot"));
    m_saveAllAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("document-save-all")), i18n("Save All Plots"));
    m_renameAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename Plot"));
    m_deleteAction = m_toolbar->addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete Plot"));

    m_searchEdit = new QLineEdit(m_containerWidget);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(i18n("Search plots"));
    m_searchEdit->setAccessibleName(i18n("Search plots"));
    outerLayout->addWidget(m_searchEdit);

    auto* listContainer = new QWidget(m_containerWidget);
    outerLayout->addWidget(listContainer, 1);
    m_plotList = new QListWidget(listContainer);
    m_plotList->setAccessibleName(i18n("Plots in the current worksheet"));
    m_plotList->setViewMode(QListView::IconMode);
    m_plotList->setMovement(QListView::Static);
    m_plotList->setResizeMode(QListView::Adjust);
    m_plotList->setIconSize(ThumbnailSize);
    m_plotList->setGridSize(QSize(264, m_showSourceLabels ? 244 : 220));
    m_plotList->setSpacing(6);
    m_plotList->setWordWrap(true);
    m_plotList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_plotList->setUniformItemSizes(true);
    m_plotList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_plotList->setContextMenuPolicy(Qt::CustomContextMenu);

    m_emptyLabel = new QLabel(i18n("No plots in the current worksheet"), listContainer);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setWordWrap(true);

    m_layout = new QStackedLayout(listContainer);
    m_layout->setContentsMargins({});
    m_layout->addWidget(m_plotList);
    m_layout->addWidget(m_emptyLabel);
    m_layout->setCurrentWidget(m_emptyLabel);

    m_animationRefreshTimer = new QTimer(this);
    m_animationRefreshTimer->setInterval(AnimationRefreshInterval);

    connect(m_plotList, &QListWidget::itemActivated, this, &PlotPanelPlugin::activatePlot);
    connect(m_plotList, &QListWidget::currentItemChanged, this, [this]() { updateCurrentPlot(); });
    connect(m_plotList, &QListWidget::customContextMenuRequested, this, &PlotPanelPlugin::showContextMenu);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PlotPanelPlugin::updateFilter);
    connect(m_animationRefreshTimer, &QTimer::timeout, this, &PlotPanelPlugin::flushAnimationFrames);
    connect(m_previousAction, &QAction::triggered, this, &PlotPanelPlugin::selectPreviousPlot);
    connect(m_nextAction, &QAction::triggered, this, &PlotPanelPlugin::selectNextPlot);
    connect(m_sourceLabelsAction, &QAction::toggled, this, &PlotPanelPlugin::updateSourceLabelMode);
    connect(m_animationAction, &QAction::toggled, this, &PlotPanelPlugin::updateAnimationMode);
    connect(m_copyAction, &QAction::triggered, this, &PlotPanelPlugin::copyCurrentPlot);
    connect(m_saveAction, &QAction::triggered, this, &PlotPanelPlugin::saveCurrentPlot);
    connect(m_saveAllAction, &QAction::triggered, this, &PlotPanelPlugin::requestSaveAllPlots);
    connect(m_renameAction, &QAction::triggered, this, &PlotPanelPlugin::renameCurrentPlot);
    connect(m_deleteAction, &QAction::triggered, this, &PlotPanelPlugin::deleteCurrentPlot);

    updateActionState();
    updateEmptyState();
}

void PlotPanelPlugin::handleTocNodesChanged(const QVariantList& nodes)
{
    if (!m_plotList)
        constructWidget();

    applyPlotNodeChanges(nodes);
}

void PlotPanelPlugin::applyPlotNodeChanges(const QVariantList& nodes)
{
    QVector<QVariantMap> plotNodes;
    QSet<QString> incomingNodeIds;
    for (const QVariant& value : nodes)
    {
        const QVariantMap node = value.toMap();
        if (node.value(QStringLiteral("type")).toString() != PlotNodeType)
            continue;
        const QString nodeId = node.value(QStringLiteral("id")).toString();
        if (nodeId.isEmpty())
            continue;
        plotNodes.append(node);
        incomingNodeIds.insert(nodeId);
    }

    const QSignalBlocker blocker(m_plotList);
    for (int row = m_plotList->count() - 1; row >= 0; --row)
    {
        auto* item = m_plotList->item(row);
        const QString nodeId = item->data(NodeIdRole).toString();
        if (incomingNodeIds.contains(nodeId))
            continue;
        m_itemsByNodeId.remove(nodeId);
        delete m_plotList->takeItem(row);
    }

    m_itemsByResultId.clear();
    for (int desiredRow = 0; desiredRow < plotNodes.size(); ++desiredRow)
    {
        const QVariantMap& node = plotNodes.at(desiredRow);
        const QString nodeId = node.value(QStringLiteral("id")).toString();
        const QString resultId = node.value(QStringLiteral("resultId")).toString();
        const QString entryId = node.value(QStringLiteral("entryId")).toString();
        const QString format = node.value(QStringLiteral("format")).toString();
        const QString sourceText = node.value(QStringLiteral("sourceText")).toString();
        const bool animated = node.value(QStringLiteral("animated")).toBool();
        const QImage preview = node.value(QStringLiteral("preview")).value<QImage>();

        auto* item = m_itemsByNodeId.value(nodeId, nullptr);
        const bool isNewItem = item == nullptr;
        if (isNewItem)
        {
            item = new QListWidgetItem;
            item->setTextAlignment(Qt::AlignHCenter);
            item->setData(NodeIdRole, nodeId);
            m_itemsByNodeId.insert(nodeId, item);
            m_plotList->insertItem(desiredRow, item);
        }
        else
        {
            const int currentRow = m_plotList->row(item);
            if (currentRow != desiredRow)
            {
                m_plotList->takeItem(currentRow);
                m_plotList->insertItem(desiredRow, item);
            }
        }

        QString title = node.value(QStringLiteral("displayText")).toString();
        if (title.isEmpty())
            title = i18n("Plot");
        item->setData(DisplayTextRole, title);
        item->setData(EntryIdRole, entryId);
        item->setData(ResultIdRole, resultId);
        item->setData(CustomTitleRole, node.value(QStringLiteral("customTitle")));
        item->setData(AnimatedRole, animated);
        item->setData(FormatRole, format);
        item->setData(SourceTextRole, sourceText);

        const bool hasLiveFrame = !isNewItem && animated && item->data(HasLiveFrameRole).toBool();
        if (!hasLiveFrame && (isNewItem || item->data(PreviewCacheKeyRole).toLongLong() != preview.cacheKey()))
        {
            item->setData(PreviewCacheKeyRole, preview.cacheKey());
            item->setIcon(previewIcon(preview));
        }
        if (!animated)
            item->setData(HasLiveFrameRole, false);

        QString toolTip = title;
        if (!sourceText.isEmpty())
            toolTip += QLatin1Char('\n') + i18n("Source: %1", sourceText);
        if (!format.isEmpty())
            toolTip += QLatin1Char('\n') + i18n("Format: %1", format);
        if (preview.isNull())
            toolTip += QLatin1Char('\n') + i18n("Preview unavailable; open the plot in the worksheet for details.");
        item->setToolTip(toolTip);
        if (!resultId.isEmpty())
            m_itemsByResultId.insert(resultId, item);
    }

    updateFilter(m_searchText);
    updateSelection();
    updateCurrentPlot();
}

void PlotPanelPlugin::clearPlots()
{
    if (m_animationRefreshTimer)
        m_animationRefreshTimer->stop();
    m_pendingAnimationFrames.clear();
    if (m_plotList)
        m_plotList->clear();
    m_itemsByNodeId.clear();
    m_itemsByResultId.clear();
}

void PlotPanelPlugin::handleCurrentTocNodeChanged(const QString& nodeId)
{
    m_currentNodeId = nodeId;
    updateSelection();
}

void PlotPanelPlugin::handleAnimationFrameChanged(const QString& resultId, const QImage& frame)
{
    auto* item = m_itemsByResultId.value(resultId, nullptr);
    const bool panelVisible = m_containerWidget && m_containerWidget->isVisible() && m_plotList && m_plotList->isVisible();
    if (!m_animatePreviews || !panelVisible || frame.isNull() || !item || item->isHidden())
        return;

    m_pendingAnimationFrames.insert(resultId, frame);
    if (!m_animationRefreshTimer->isActive())
    {
        flushAnimationFrames();
        m_animationRefreshTimer->start();
    }
}

void PlotPanelPlugin::flushAnimationFrames()
{
    if (!m_animatePreviews || !m_plotList || !m_plotList->isVisible())
    {
        m_pendingAnimationFrames.clear();
        if (m_animationRefreshTimer)
            m_animationRefreshTimer->stop();
        return;
    }

    if (m_pendingAnimationFrames.isEmpty())
    {
        m_animationRefreshTimer->stop();
        return;
    }

    const auto pendingFrames = std::exchange(m_pendingAnimationFrames, {});
    for (auto it = pendingFrames.cbegin(); it != pendingFrames.cend(); ++it)
    {
        auto* item = m_itemsByResultId.value(it.key(), nullptr);
        if (!item || item->isHidden())
            continue;

        const bool isCurrentItem = item == m_plotList->currentItem();
        const bool thumbnailVisible = m_plotList->visualItemRect(item).intersects(m_plotList->viewport()->rect());
        if (!isCurrentItem && !thumbnailVisible)
            continue;

        const QImage displayFrame = scaledPreview(it.value(), Qt::FastTransformation);
        item->setData(PreviewCacheKeyRole, displayFrame.cacheKey());
        item->setData(HasLiveFrameRole, true);
        item->setIcon(previewIcon(displayFrame, Qt::FastTransformation));
    }
}

void PlotPanelPlugin::handleReadOnlyChanged(bool readOnly)
{
    m_readOnly = readOnly;
    updateActionState();
}

void PlotPanelPlugin::updateFilter(const QString& text)
{
    if (!m_plotList)
        return;

    m_searchText = text;
    for (int row = 0; row < m_plotList->count(); ++row)
    {
        auto* item = m_plotList->item(row);
        const bool titleMatches = item->data(DisplayTextRole).toString().contains(text, Qt::CaseInsensitive);
        const bool sourceMatches = item->data(SourceTextRole).toString().contains(text, Qt::CaseInsensitive);
        const bool formatMatches = item->data(FormatRole).toString().contains(text, Qt::CaseInsensitive);
        const bool matches = text.isEmpty() || titleMatches || sourceMatches || formatMatches;
        item->setHidden(!matches);
    }

    updateItemLabels();
    if (!m_plotList->currentItem() || m_plotList->currentItem()->isHidden())
        m_plotList->setCurrentItem(firstVisibleItem());
    updateEmptyState();
    updateActionState();
}

void PlotPanelPlugin::updateEmptyState()
{
    if (!m_plotList || !m_layout)
        return;

    int visibleCount = 0;
    for (int row = 0; row < m_plotList->count(); ++row)
        visibleCount += !m_plotList->item(row)->isHidden();

    if (visibleCount == 0)
    {
        m_emptyLabel->setText(m_plotList->count() == 0 ? i18n("No plots in the current worksheet") : i18n("No plots match the search"));
        m_layout->setCurrentWidget(m_emptyLabel);
    }
    else
        m_layout->setCurrentWidget(m_plotList);

    if (m_searchEdit)
        m_searchEdit->setEnabled(m_plotList->count() > 0);
}

void PlotPanelPlugin::updateSelection()
{
    if (!m_plotList)
        return;

    auto* item = m_itemsByNodeId.value(m_currentNodeId, nullptr);
    if (item && !item->isHidden())
    {
        m_plotList->setCurrentItem(item);
        m_plotList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }
    else if (!m_plotList->currentItem() || m_plotList->currentItem()->isHidden())
        m_plotList->setCurrentItem(firstVisibleItem());
}

QListWidgetItem* PlotPanelPlugin::currentPlotItem() const
{
    return m_plotList ? m_plotList->currentItem() : nullptr;
}

QListWidgetItem* PlotPanelPlugin::firstVisibleItem() const
{
    if (!m_plotList)
        return nullptr;
    for (int row = 0; row < m_plotList->count(); ++row)
    {
        auto* item = m_plotList->item(row);
        if (!item->isHidden())
            return item;
    }
    return nullptr;
}

void PlotPanelPlugin::updateCurrentPlot()
{
    updateActionState();
}

void PlotPanelPlugin::updateActionState()
{
    if (!m_plotList)
        return;

    const bool hasCurrent = currentPlotItem() != nullptr;
    int visibleCount = 0;
    bool hasAnimation = false;
    for (int row = 0; row < m_plotList->count(); ++row)
    {
        auto* item = m_plotList->item(row);
        visibleCount += !item->isHidden();
        hasAnimation = hasAnimation || item->data(AnimatedRole).toBool();
    }

    m_previousAction->setEnabled(visibleCount > 1);
    m_nextAction->setEnabled(visibleCount > 1);
    m_sourceLabelsAction->setEnabled(m_plotList->count() > 0);
    m_copyAction->setEnabled(hasCurrent);
    m_saveAction->setEnabled(hasCurrent);
    m_saveAllAction->setEnabled(m_plotList->count() > 0);
    m_renameAction->setEnabled(hasCurrent && !m_readOnly);
    m_deleteAction->setEnabled(hasCurrent && !m_readOnly);
    m_animationAction->setEnabled(hasAnimation);
}

void PlotPanelPlugin::updateItemLabels()
{
    if (!m_plotList)
        return;

    QString previousSource;
    for (int row = 0; row < m_plotList->count(); ++row)
    {
        auto* item = m_plotList->item(row);
        const QString title = item->data(DisplayTextRole).toString();
        const QString source = item->data(SourceTextRole).toString();
        if (!item->isHidden() && m_showSourceLabels && !source.isEmpty() && source != previousSource)
            item->setText(source + QLatin1Char('\n') + title);
        else
            item->setText(title);
        if (!item->isHidden())
            previousSource = source;
    }
}

void PlotPanelPlugin::activatePlot(QListWidgetItem* item)
{
    if (!item)
        return;

    const QString nodeId = item->data(NodeIdRole).toString();
    if (!nodeId.isEmpty())
        Q_EMIT requestNavigateToTocNode(nodeId);
}

void PlotPanelPlugin::selectRelativePlot(int direction)
{
    if (!m_plotList || direction == 0)
        return;

    QVector<QListWidgetItem*> visibleItems;
    for (int row = 0; row < m_plotList->count(); ++row)
    {
        auto* item = m_plotList->item(row);
        if (!item->isHidden())
            visibleItems.append(item);
    }
    if (visibleItems.isEmpty())
        return;

    int index = visibleItems.indexOf(currentPlotItem());
    if (index < 0)
        index = direction > 0 ? -1 : 0;
    index = (index + direction + visibleItems.size()) % visibleItems.size();
    auto* item = visibleItems.at(index);
    m_plotList->setCurrentItem(item);
    m_plotList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    activatePlot(item);
}

void PlotPanelPlugin::selectPreviousPlot()
{
    selectRelativePlot(-1);
}

void PlotPanelPlugin::selectNextPlot()
{
    selectRelativePlot(1);
}

void PlotPanelPlugin::renameCurrentPlot()
{
    auto* item = currentPlotItem();
    if (!item || m_readOnly)
        return;

    bool accepted = false;
    const QString currentTitle = item->data(CustomTitleRole).toString();
    const QString dialogTitle = i18n("Rename Plot");
    const QString label = i18n("Plot name (leave empty to use the default name):");
    const QString title = QInputDialog::getText(m_containerWidget, dialogTitle, label, QLineEdit::Normal, currentTitle, &accepted);
    if (accepted && title.trimmed() != currentTitle)
        Q_EMIT requestRenamePlot(item->data(EntryIdRole).toString(), item->data(ResultIdRole).toString(), title);
}

void PlotPanelPlugin::deleteCurrentPlot()
{
    auto* item = currentPlotItem();
    if (!item || m_readOnly)
        return;

    const QString question = i18n("Do you really want to delete \"%1\"? This action cannot be undone.", item->data(DisplayTextRole).toString());
    const auto buttons = QMessageBox::Yes | QMessageBox::Cancel;
    const auto answer = QMessageBox::question(m_containerWidget, i18n("Delete Plot"), question, buttons, QMessageBox::Cancel);
    if (answer == QMessageBox::Yes)
        Q_EMIT requestDeletePlot(item->data(EntryIdRole).toString(), item->data(ResultIdRole).toString());
}

void PlotPanelPlugin::saveCurrentPlot()
{
    auto* item = currentPlotItem();
    if (item)
        Q_EMIT requestSavePlot(item->data(EntryIdRole).toString(), item->data(ResultIdRole).toString());
}

void PlotPanelPlugin::copyCurrentPlot()
{
    auto* item = currentPlotItem();
    if (item)
        Q_EMIT requestCopyPlot(item->data(EntryIdRole).toString(), item->data(ResultIdRole).toString());
}

void PlotPanelPlugin::updateAnimationMode(bool enabled)
{
    m_animatePreviews = enabled;
    if (!enabled)
    {
        m_pendingAnimationFrames.clear();
        if (m_animationRefreshTimer)
            m_animationRefreshTimer->stop();
    }
}

void PlotPanelPlugin::updateSourceLabelMode(bool enabled)
{
    m_showSourceLabels = enabled;
    if (m_plotList)
        m_plotList->setGridSize(QSize(264, enabled ? 244 : 220));
    updateItemLabels();
}

void PlotPanelPlugin::showContextMenu(const QPoint& position)
{
    if (!m_plotList)
        return;
    if (auto* item = m_plotList->itemAt(position))
        m_plotList->setCurrentItem(item);

    QMenu menu(m_plotList);
    auto* openAction = menu.addAction(QIcon::fromTheme(QStringLiteral("go-jump")), i18n("Open in Worksheet"));
    connect(openAction, &QAction::triggered, this, [this]() { activatePlot(currentPlotItem()); });
    menu.addAction(m_previousAction);
    menu.addAction(m_nextAction);
    menu.addSeparator();
    menu.addAction(m_sourceLabelsAction);
    menu.addAction(m_animationAction);
    menu.addSeparator();
    menu.addAction(m_copyAction);
    menu.addAction(m_saveAction);
    menu.addAction(m_saveAllAction);
    menu.addSeparator();
    menu.addAction(m_renameAction);
    menu.addAction(m_deleteAction);
    menu.exec(m_plotList->viewport()->mapToGlobal(position));
}

K_PLUGIN_FACTORY_WITH_JSON(plotpanelplugin, "plotpanelplugin.json", registerPlugin<PlotPanelPlugin>();)
#include "plotpanelplugin.moc"
