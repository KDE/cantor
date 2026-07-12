#include "worksheethierarchymanager.h"

#include "commandentry.h"
#include "hierarchyentry.h"
#include "placeholderentry.h"
#include "resultitem.h"
#include "settings.h"
#include "worksheet.h"
#include "worksheetentry.h"
#include "worksheettextitem.h"
#include "worksheetview.h"
#include "lib/animationresult.h"
#include "lib/expression.h"
#include "lib/imageresult.h"
#include "lib/pdfresult.h"
#include "lib/result.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <QList>
#include <QRectF>
#include <QSet>
#include <QTimer>
#include <QVariantMap>

#include <algorithm>

namespace
{
    const QLatin1String TocNodeTypeChapter("chapter");
    const QLatin1String TocNodeTypeSection("section");
    const QLatin1String TocNodeTypeCommand("command");
    const QLatin1String TocNodeTypePlot("plot");
}

WorksheetHierarchyManager::WorksheetHierarchyManager(Worksheet* worksheet)
    : QObject(worksheet)
    , m_worksheet(worksheet)
{
    Q_ASSERT(m_worksheet);
}

size_t WorksheetHierarchyManager::hierarchyMaxDepth() const
{
    return m_hierarchyMaxDepth;
}

void WorksheetHierarchyManager::refreshTocStructure()
{
    if (m_worksheet->m_isClosing || m_worksheet->m_isLoadingFromFile)
        return;

    m_tocRefreshScheduled = false;
    m_tocNodeSnapshot = collectTocNodes();

    if (!m_currentTocNodeId.isEmpty())
    {
        bool currentNodeStillExists = false;
        for (const QVariant& nodeValue : m_tocNodeSnapshot)
        {
            if (nodeValue.toMap().value(QStringLiteral("id")).toString() == m_currentTocNodeId)
            {
                currentNodeStillExists = true;
                break;
            }
        }

        if (!currentNodeStillExists)
            setCurrentTocNode(QString());
    }

    Q_EMIT m_worksheet->tocNodesChanged(m_tocNodeSnapshot);
}

void WorksheetHierarchyManager::scheduleTocStructureRefresh()
{
    if (m_worksheet->m_isClosing || m_worksheet->m_isLoadingFromFile || m_tocRefreshScheduled)
        return;

    m_tocRefreshScheduled = true;
    QTimer::singleShot(0, this, [this]()
    {
        if (!m_tocRefreshScheduled)
            return;

        refreshTocStructure();
    });
}

void WorksheetHierarchyManager::emitTocNodeSnapshot()
{
    if (m_worksheet->m_isClosing)
        return;

    if (m_tocNodeSnapshot.isEmpty() && m_worksheet->firstEntry())
        m_tocNodeSnapshot = collectTocNodes();

    Q_EMIT m_worksheet->tocNodesChanged(m_tocNodeSnapshot);
}

QVariantList WorksheetHierarchyManager::collectTocNodes() const
{
    QVariantList nodes;
    QVector<QString> hierarchyNodeIds;
    QVector<int> hierarchyDepths;

    visitLogicalEntries([&](WorksheetEntry* entry)
    {
        if (entry->type() == HierarchyEntry::Type)
        {
            auto* hierarchyEntry = static_cast<HierarchyEntry*>(entry);
            const int depth = static_cast<int>(hierarchyEntry->level()) - static_cast<int>(HierarchyEntry::HierarchyLevel::Chapter);

            while (!hierarchyDepths.isEmpty() && hierarchyDepths.last() >= depth)
            {
                hierarchyDepths.removeLast();
                hierarchyNodeIds.removeLast();
            }

            const QString parentNodeId = hierarchyNodeIds.isEmpty() ? QString() : hierarchyNodeIds.last();
            const QString hierarchyId = hierarchyEntry->hierarchyId();
            const QString title = hierarchyEntry->text();
            const QString displayText = hierarchyEntry->hierarchyText().isEmpty()
                ? hierarchyEntry->text()
                : hierarchyEntry->hierarchyText() + QLatin1Char(' ') + hierarchyEntry->text();

            QVariantMap node;
            node.insert(QStringLiteral("id"), hierarchyId);
            node.insert(QStringLiteral("parentId"), parentNodeId);
            node.insert(QStringLiteral("type"), depth == 0 ? QString(TocNodeTypeChapter) : QString(TocNodeTypeSection));
            node.insert(QStringLiteral("title"), title);
            node.insert(QStringLiteral("displayText"), displayText);
            node.insert(QStringLiteral("hierarchyText"), hierarchyEntry->hierarchyText());
            node.insert(QStringLiteral("depth"), depth);
            node.insert(QStringLiteral("editable"), true);
            node.insert(QStringLiteral("navigable"), true);
            node.insert(QStringLiteral("hierarchyId"), hierarchyId);
            node.insert(QStringLiteral("resultIndex"), -1);
            node.insert(QStringLiteral("entryId"), hierarchyId);
            nodes.append(node);

            hierarchyNodeIds.append(hierarchyId);
            hierarchyDepths.append(depth);
        }
        else if (entry->type() == CommandEntry::Type)
        {
            auto* commandEntry = static_cast<CommandEntry*>(entry);
            const QString parentNodeId = hierarchyNodeIds.isEmpty() ? QString() : hierarchyNodeIds.last();
            const QString commandNodeId = buildCommandNodeId(commandEntry);

            QVariantMap commandNode;
            commandNode.insert(QStringLiteral("id"), commandNodeId);
            commandNode.insert(QStringLiteral("parentId"), parentNodeId);
            commandNode.insert(QStringLiteral("type"), QString(TocNodeTypeCommand));
            commandNode.insert(QStringLiteral("title"), commandTocTitle());
            commandNode.insert(QStringLiteral("displayText"), commandTocDisplayText(commandEntry));
            commandNode.insert(QStringLiteral("hierarchyText"), QString());
            commandNode.insert(QStringLiteral("depth"), hierarchyDepths.isEmpty() ? 0 : hierarchyDepths.last() + 1);
            commandNode.insert(QStringLiteral("editable"), false);
            commandNode.insert(QStringLiteral("navigable"), true);
            commandNode.insert(QStringLiteral("hierarchyId"), QString());
            commandNode.insert(QStringLiteral("resultIndex"), -1);
            commandNode.insert(QStringLiteral("entryId"), commandEntry->commandId());
            nodes.append(commandNode);

            if (auto* expression = commandEntry->expression())
            {
                const auto& results = expression->results();
                int plotCount = 0;
                for (auto* result : results)
                {
                    if (isPlotResult(result))
                        ++plotCount;
                }

                int plotOrdinal = 0;
                for (int index = 0; index < results.size(); ++index)
                {
                    auto* result = results.at(index);
                    if (!isPlotResult(result))
                        continue;

                    ++plotOrdinal;

                    QVariantMap plotNode;
                    const QString customTitle = result->displayName();
                    plotNode.insert(QStringLiteral("id"), buildPlotNodeId(commandEntry->commandId(), result->resultId()));
                    plotNode.insert(QStringLiteral("parentId"), commandNodeId);
                    plotNode.insert(QStringLiteral("type"), QString(TocNodeTypePlot));
                    plotNode.insert(QStringLiteral("title"), plotTocTitle(result));
                    plotNode.insert(QStringLiteral("customTitle"), customTitle);
                    plotNode.insert(QStringLiteral("displayText"), plotTocDisplayText(commandEntry, result, plotOrdinal, plotCount));
                    plotNode.insert(QStringLiteral("hierarchyText"), QString());
                    plotNode.insert(QStringLiteral("depth"), hierarchyDepths.isEmpty() ? 1 : hierarchyDepths.last() + 2);
                    plotNode.insert(QStringLiteral("editable"), true);
                    plotNode.insert(QStringLiteral("navigable"), true);
                    plotNode.insert(QStringLiteral("hierarchyId"), QString());
                    plotNode.insert(QStringLiteral("resultIndex"), index);
                    plotNode.insert(QStringLiteral("resultId"), result->resultId());
                    plotNode.insert(QStringLiteral("entryId"), commandEntry->commandId());
                    nodes.append(plotNode);
                }
            }
        }
        return true;
    });

    return nodes;
}

QString WorksheetHierarchyManager::buildCommandNodeId(CommandEntry* entry) const
{
    if (!entry)
        return QString();

    const QString& commandId = entry->commandId();
    if (commandId.isEmpty())
        return QString();

    return QStringLiteral("command:") + commandId;
}

QString WorksheetHierarchyManager::buildPlotNodeId(const QString& commandId, const QString& resultId) const
{
    if (commandId.isEmpty() || resultId.isEmpty())
        return QString();

    return QStringLiteral("plot:%1:%2").arg(commandId, resultId);
}

bool WorksheetHierarchyManager::parseCommandNodeId(const QString& nodeId, QString* commandId) const
{
    const QString prefix = QStringLiteral("command:");
    if (!nodeId.startsWith(prefix))
        return false;

    const QString parsedCommandId = nodeId.mid(prefix.size());
    if (parsedCommandId.isEmpty() || parsedCommandId.contains(QLatin1Char(':')))
        return false;

    if (commandId)
        *commandId = parsedCommandId;

    return true;
}

bool WorksheetHierarchyManager::parsePlotNodeId(const QString& nodeId, QString* commandId, QString* resultId) const
{
    const QString prefix = QStringLiteral("plot:");
    if (!nodeId.startsWith(prefix))
        return false;

    const QStringList parts = nodeId.mid(prefix.size()).split(QLatin1Char(':'));
    if (parts.size() != 2 || parts.at(0).isEmpty() || parts.at(1).isEmpty())
        return false;

    if (commandId)
        *commandId = parts.at(0);
    if (resultId)
        *resultId = parts.at(1);

    return true;
}

QString WorksheetHierarchyManager::commandTocTitle() const
{
    return i18n("Command");
}

QString WorksheetHierarchyManager::commandTocDisplayText(CommandEntry* entry) const
{
    auto* expression = entry ? entry->expression() : nullptr;
    const QString title = commandTocTitle();
    if (m_worksheet->m_showExpressionIds && expression && expression->id() != -1)
        return i18n("%1 %2", title, expression->id());

    return title;
}

QString WorksheetHierarchyManager::plotTocTitle(Cantor::Result* result) const
{
    if (result && !result->displayName().isEmpty())
        return result->displayName();

    return i18n("Plot");
}

QString WorksheetHierarchyManager::plotTocDisplayText(CommandEntry* entry, Cantor::Result* result, int plotOrdinal, int plotCount) const
{
    const QString title = plotTocTitle(result);
    auto* expression = entry ? entry->expression() : nullptr;
    if (m_worksheet->m_showExpressionIds && expression && expression->id() != -1)
    {
        if (plotCount > 1)
            return i18n("%1 %2.%3", title, expression->id(), plotOrdinal);

        return i18n("%1 %2", title, expression->id());
    }

    if (plotCount > 1)
        return i18n("%1 %2", title, plotOrdinal);

    return title;
}

bool WorksheetHierarchyManager::isPlotResult(Cantor::Result* result) const
{
    if (!result || result->role() != Cantor::Result::Role::Plot)
        return false;

    return result->type() == Cantor::ImageResult::Type
        || result->type() == Cantor::AnimationResult::Type
        || result->type() == Cantor::PdfResult::Type;
}

bool WorksheetHierarchyManager::visitLogicalEntries(WorksheetEntry* first, const std::function<bool(WorksheetEntry*)>& visitor) const
{
    for (auto* entry = first; entry; entry = entry->next())
    {
        if (!visitor(entry))
            return false;

        if (entry->type() != HierarchyEntry::Type)
            continue;

        auto* hierarchyEntry = static_cast<HierarchyEntry*>(entry);
        if (auto* hiddenEntry = hierarchyEntry->hiddenSubentries())
        {
            if (!visitLogicalEntries(hiddenEntry, visitor))
                return false;
        }
    }

    return true;
}

bool WorksheetHierarchyManager::visitLogicalEntries(const std::function<bool(WorksheetEntry*)>& visitor) const
{
    return visitLogicalEntries(m_worksheet->firstEntry(), visitor);
}

bool WorksheetHierarchyManager::findHierarchyEntryById(WorksheetEntry* first, const QString& hierarchyId, QVector<HierarchyEntry*> collapsedAncestors, HierarchySearchResult& result) const
{
    for (auto* entry = first; entry; entry = entry->next())
    {
        if (entry->type() != HierarchyEntry::Type)
            continue;

        auto* hierarchyEntry = static_cast<HierarchyEntry*>(entry);

        if (hierarchyEntry->hierarchyId() == hierarchyId)
        {
            result.entry = hierarchyEntry;
            result.collapsedAncestors = collapsedAncestors;
            return true;
        }

        if (auto* hiddenEntry = hierarchyEntry->hiddenSubentries())
        {
            auto childAncestors = collapsedAncestors;
            childAncestors.append(hierarchyEntry);

            if (findHierarchyEntryById(hiddenEntry, hierarchyId, childAncestors, result))
                return true;
        }
    }

    return false;
}

WorksheetHierarchyManager::HierarchySearchResult WorksheetHierarchyManager::findHierarchyEntryById(const QString& hierarchyId) const
{
    HierarchySearchResult result;

    if (hierarchyId.isEmpty())
        return result;

    findHierarchyEntryById(m_worksheet->firstEntry(), hierarchyId, {}, result);
    return result;
}

bool WorksheetHierarchyManager::findCommandEntryById(WorksheetEntry* first, const QString& commandId, QVector<HierarchyEntry*> collapsedAncestors, CommandSearchResult& result) const
{
    for (auto* entry = first; entry; entry = entry->next())
    {
        if (entry->type() == CommandEntry::Type)
        {
            auto* commandEntry = static_cast<CommandEntry*>(entry);
            if (commandEntry->commandId() == commandId)
            {
                result.entry = commandEntry;
                result.collapsedAncestors = collapsedAncestors;
                return true;
            }
        }

        if (entry->type() != HierarchyEntry::Type)
            continue;

        auto* hierarchyEntry = static_cast<HierarchyEntry*>(entry);
        if (auto* hiddenEntry = hierarchyEntry->hiddenSubentries())
        {
            auto childAncestors = collapsedAncestors;
            childAncestors.append(hierarchyEntry);

            if (findCommandEntryById(hiddenEntry, commandId, childAncestors, result))
                return true;
        }
    }

    return false;
}

WorksheetHierarchyManager::CommandSearchResult WorksheetHierarchyManager::findCommandEntryById(const QString& commandId) const
{
    CommandSearchResult result;

    if (commandId.isEmpty())
        return result;

    findCommandEntryById(m_worksheet->firstEntry(), commandId, {}, result);
    return result;
}

bool WorksheetHierarchyManager::expandHierarchyAncestors(const QVector<HierarchyEntry*>& ancestors)
{
    bool expanded = false;

    for (auto* ancestor : ancestors)
    {
        if (!ancestor || !ancestor->hasHiddenSubentries())
            continue;

        WorksheetEntry* hiddenSubentries = ancestor->takeHiddenSubentries();

        if (!hiddenSubentries)
            continue;

        insertSubentriesForHierarchy(ancestor, hiddenSubentries);
        expanded = true;
    }

    return expanded;
}

void WorksheetHierarchyManager::updateHierarchyLayout()
{
    QSet<QString> usedHierarchyIds;
    QSet<QString> usedCommandIds;
    QSet<QString> usedResultIds;

    m_hierarchyMaxDepth = 0;
    std::vector<int> hierarchyNumbers;

    visitLogicalEntries([&](WorksheetEntry* entry)
    {
        if (entry->type() == HierarchyEntry::Type)
        {
            auto* hierarchyEntry = static_cast<HierarchyEntry*>(entry);

            hierarchyEntry->updateHierarchyLevel(hierarchyNumbers);

            m_hierarchyMaxDepth = std::max(m_hierarchyMaxDepth, hierarchyNumbers.size());

            // Keep IDs valid for old files and duplicated entries.
            QString hierarchyId = hierarchyEntry->hierarchyId();

            if (hierarchyId.isEmpty() || usedHierarchyIds.contains(hierarchyId))
            {
                hierarchyEntry->regenerateHierarchyId();
                hierarchyId = hierarchyEntry->hierarchyId();
            }

            usedHierarchyIds.insert(hierarchyId);
        }
        else if (entry->type() == CommandEntry::Type)
        {
            auto* commandEntry = static_cast<CommandEntry*>(entry);
            QString commandId = commandEntry->commandId();

            if (commandId.isEmpty() || usedCommandIds.contains(commandId))
            {
                commandEntry->regenerateCommandId();
                commandId = commandEntry->commandId();
            }

            usedCommandIds.insert(commandId);

            if (auto* expression = commandEntry->expression())
            {
                const auto& results = expression->results();
                for (auto* result : results)
                {
                    if (!result)
                        continue;

                    QString resultId = result->resultId();
                    if (resultId.isEmpty() || usedResultIds.contains(resultId))
                    {
                        result->regenerateResultId();
                        resultId = result->resultId();
                    }

                    usedResultIds.insert(resultId);
                }
            }
        }

        return true;
    });

    refreshTocStructure();
}

void WorksheetHierarchyManager::updateHierarchyControlsLayout(WorksheetEntry* startEntry)
{
    Q_UNUSED(startEntry);

    std::vector<HierarchyEntry*> levelEntries;
    const int numerationBegin = static_cast<int>(HierarchyEntry::HierarchyLevel::Chapter);
    const int numerationEnd = static_cast<int>(HierarchyEntry::HierarchyLevel::EndValue);

    for (int i = numerationBegin; i < numerationEnd; ++i)
        levelEntries.push_back(nullptr);

    WorksheetEntry* lastRealEntry = nullptr;

    for (auto* entry = m_worksheet->firstEntry(); entry; entry = entry->next())
    {
        if (entry->type() == PlaceHolderEntry::Type || entry->aboutToBeRemoved())
            continue;

        lastRealEntry = entry;

        if (entry->type() != HierarchyEntry::Type)
            continue;

        auto* hierarchyEntry = static_cast<HierarchyEntry*>(entry);

        const int index = static_cast<int>(hierarchyEntry->level()) - numerationBegin;

        if (index < 0 || index >= static_cast<int>(levelEntries.size()))
            continue;

        if (!levelEntries[index])
        {
            levelEntries[index] = hierarchyEntry;
            continue;
        }

        // Close previous controls at this level and below.
        for (int i = index; i < static_cast<int>(levelEntries.size()); ++i)
        {
            auto* openEntry = levelEntries[i];

            if (!openEntry)
                continue;

            const qreal ownBottom = openEntry->y() + openEntry->size().height() - WorksheetEntry::VerticalMargin;
            const qreal controlEnd = hierarchyEntry->y() - WorksheetEntry::VerticalMargin;
            const bool hasSubelements = controlEnd > ownBottom;

            openEntry->updateControlElementForHierarchy(qMax(controlEnd, ownBottom), m_hierarchyMaxDepth, hasSubelements);

            levelEntries[i] = nullptr;
        }

        levelEntries[index] = hierarchyEntry;
    }

    if (!lastRealEntry)
        return;

    const qreal documentEnd = lastRealEntry->y() + lastRealEntry->size().height() - WorksheetEntry::VerticalMargin;

    for (auto* openEntry : levelEntries)
    {
        if (!openEntry)
            continue;

        const qreal ownBottom = openEntry->y() + openEntry->size().height() - WorksheetEntry::VerticalMargin;
        const qreal controlEnd = qMax(documentEnd, ownBottom);
        const bool hasSubelements = controlEnd > ownBottom;

        openEntry->updateControlElementForHierarchy(controlEnd, m_hierarchyMaxDepth, hasSubelements);
    }
}

std::vector<WorksheetEntry*> WorksheetHierarchyManager::hierarchySubelements(HierarchyEntry* hierarchyEntry) const
{
    std::vector<WorksheetEntry*> subentries;

    Q_ASSERT(hierarchyEntry);

    bool subentriesEnd = false;
    const int level = (int)hierarchyEntry->level();
    for (auto* entry = hierarchyEntry->next(); entry && !subentriesEnd; entry = entry->next())
    {
        if (entry->type() == HierarchyEntry::Type)
        {
            if ((int)(static_cast<HierarchyEntry*>(entry)->level()) <= level)
                subentriesEnd = true;
            else
                subentries.push_back(entry);
        }
        else
            subentries.push_back(entry);
    }
    return subentries;
}

WorksheetEntry* WorksheetHierarchyManager::cutSubentriesForHierarchy(HierarchyEntry* hierarchyEntry)
{
    if (!hierarchyEntry || !hierarchyEntry->next())
        return nullptr;

    WorksheetEntry* cutBegin = hierarchyEntry->next();
    if (cutBegin->type() == HierarchyEntry::Type && static_cast<int>(static_cast<HierarchyEntry*>(cutBegin)->level()) <= static_cast<int>(hierarchyEntry->level()))
        return nullptr;
    WorksheetEntry* cutEnd = cutBegin;

    const int hierarchyLevel = static_cast<int>(hierarchyEntry->level());

    while (cutEnd->next())
    {
        WorksheetEntry* candidate = cutEnd->next();

        if (candidate->type() == HierarchyEntry::Type)
        {
            const int candidateLevel = static_cast<int>(static_cast<HierarchyEntry*>(candidate)->level());

            if (candidateLevel <= hierarchyLevel)
                break;
        }

        cutEnd = candidate;
    }

    WorksheetEntry* entryAfterSection = cutEnd->next();

    hierarchyEntry->setNext(entryAfterSection);

    if (entryAfterSection)
        entryAfterSection->setPrevious(hierarchyEntry);
    else
        m_worksheet->setLastEntry(hierarchyEntry);

    cutBegin->setPrevious(nullptr);
    cutEnd->setNext(nullptr);

    for (auto* entry = cutBegin; entry; entry = entry->next())
        entry->hide();

    return cutBegin;
}

void WorksheetHierarchyManager::insertSubentriesForHierarchy(HierarchyEntry* hierarchyEntry, WorksheetEntry* storedSubentriesBegin)
{
    if (!hierarchyEntry || !storedSubentriesBegin)
        return;

    WorksheetEntry* previousNext = hierarchyEntry->next();

    hierarchyEntry->setNext(storedSubentriesBegin);
    storedSubentriesBegin->setPrevious(hierarchyEntry);

    WorksheetEntry* storedEnd = storedSubentriesBegin;

    for (auto* entry = storedSubentriesBegin; entry; entry = entry->next())
    {
        entry->show();
        storedEnd = entry;
    }

    storedEnd->setNext(previousNext);

    if (previousNext)
        previousNext->setPrevious(storedEnd);
    else
        m_worksheet->setLastEntry(storedEnd);
}

bool WorksheetHierarchyManager::expandHierarchyForStructureChange(HierarchyEntry* hierarchyEntry)
{
    if (!hierarchyEntry)
        return false;

    bool hierarchyExpanded = false;
    const auto expandEntry = [this, &hierarchyExpanded](HierarchyEntry* entry)
    {
        if (!entry || !entry->hasHiddenSubentries())
            return;

        WorksheetEntry* hiddenSubentries = entry->takeHiddenSubentries();

        if (!hiddenSubentries)
            return;

        insertSubentriesForHierarchy(entry, hiddenSubentries);

        hierarchyExpanded = true;
    };

    expandEntry(hierarchyEntry);

    const int rootLevel = static_cast<int>(hierarchyEntry->level());
    for (auto* entry = hierarchyEntry->next(); entry;)
    {
        if (entry->type() == HierarchyEntry::Type)
        {
            auto* childHierarchy = static_cast<HierarchyEntry*>(entry);
            const int childLevel = static_cast<int>(childHierarchy->level());

            if (childLevel <= rootLevel)
                break;

            expandEntry(childHierarchy);
        }

        entry = entry->next();
    }

    return hierarchyExpanded;
}

void WorksheetHierarchyManager::navigateToTocNode(QString nodeId)
{
    QString plotCommandId;
    QString plotResultId;
    if (parsePlotNodeId(nodeId, &plotCommandId, &plotResultId))
    {
        const CommandSearchResult commandSearch = findCommandEntryById(plotCommandId);
        if (!commandSearch.entry)
        {
            scheduleTocStructureRefresh();
            return;
        }

        const bool expanded = expandHierarchyAncestors(commandSearch.collapsedAncestors);
        if (expanded)
        {
            updateHierarchyLayout();
            m_worksheet->updateLayout();
        }

        if (!navigateToPlotResult(commandSearch.entry, plotResultId))
            scheduleTocStructureRefresh();
        return;
    }

    QString commandId;
    if (parseCommandNodeId(nodeId, &commandId))
    {
        const CommandSearchResult commandSearch = findCommandEntryById(commandId);
        if (!commandSearch.entry)
            return;

        const bool expanded = expandHierarchyAncestors(commandSearch.collapsedAncestors);
        if (expanded)
        {
            updateHierarchyLayout();
            m_worksheet->updateLayout();
        }

        auto* commandEntry = commandSearch.entry;
        updateCurrentHierarchyFromEntry(commandEntry);

        m_worksheet->worksheetView()->scrollTo(qRound(commandEntry->scenePos().y()));
        m_worksheet->worksheetView()->setFocus();
        commandEntry->focusEntry(WorksheetTextItem::TopLeft);

        m_worksheet->resetEntryCursor();
        return;
    }

    const HierarchySearchResult hierarchySearch = findHierarchyEntryById(nodeId);
    auto* hierarchyEntry = hierarchySearch.entry;

    if (hierarchyEntry)
    {
        const bool expanded = expandHierarchyAncestors(hierarchySearch.collapsedAncestors);
        if (expanded)
        {
            updateHierarchyLayout();
            m_worksheet->updateLayout();
        }

        updateCurrentHierarchyFromEntry(hierarchyEntry);

        m_worksheet->worksheetView()->scrollTo(qRound(hierarchyEntry->scenePos().y()));
        m_worksheet->worksheetView()->setFocus();
        hierarchyEntry->focusEntry(WorksheetTextItem::BottomRight);

        m_worksheet->resetEntryCursor();
    }
}

bool WorksheetHierarchyManager::navigateToPlotResult(CommandEntry* commandEntry, const QString& resultId)
{
    if (!commandEntry || resultId.isEmpty())
        return false;

    if (commandEntry->isResultCollapsed())
    {
        commandEntry->expandResults();
        m_worksheet->updateLayout();
    }

    auto* resultItem = commandEntry->resultItemById(resultId);
    if (!resultItem || !resultItem->result() || !isPlotResult(resultItem->result()))
        return false;

    QGraphicsObject* object = resultItem->graphicsObject();
    if (!object)
        return false;

    const QString nodeId = buildPlotNodeId(commandEntry->commandId(), resultId);

    m_worksheet->worksheetView()->scrollTo(qRound(object->sceneBoundingRect().top()));
    m_worksheet->worksheetView()->setFocus();
    object->setFocus();

    m_trackingSource = TrackingSource::FocusedEntry;
    setCurrentTocNode(nodeId);

    m_worksheet->resetEntryCursor();
    return true;
}

void WorksheetHierarchyManager::updateCurrentTocNodeFromResult(CommandEntry* commandEntry, Cantor::Result* result)
{
    if (!commandEntry || !result || !m_worksheet->isValidEntry(commandEntry))
        return;

    m_trackingSource = TrackingSource::FocusedEntry;

    if (isPlotResult(result))
        setCurrentTocNode(buildPlotNodeId(commandEntry->commandId(), result->resultId()));
    else
        updateCurrentHierarchyFromEntry(commandEntry);
}

void WorksheetHierarchyManager::renamePlot(const QString& commandId, const QString& resultId, const QString& newTitle)
{
    if (m_worksheet->m_readOnly || commandId.isEmpty() || resultId.isEmpty())
        return;

    QString normalizedTitle = newTitle;
    normalizedTitle.replace(QLatin1Char('\r'), QLatin1Char(' '));
    normalizedTitle.replace(QLatin1Char('\n'), QLatin1Char(' '));
    normalizedTitle = normalizedTitle.trimmed();

    const CommandSearchResult commandSearch = findCommandEntryById(commandId);
    auto* commandEntry = commandSearch.entry;
    auto* expression = commandEntry ? commandEntry->expression() : nullptr;

    if (!commandEntry || !expression)
        return;

    for (auto* result : expression->results())
    {
        if (!result || result->resultId() != resultId || !isPlotResult(result))
            continue;

        if (result->displayName() == normalizedTitle)
            return;

        result->setDisplayName(normalizedTitle);
        m_worksheet->setModified();
        scheduleTocStructureRefresh();
        return;
    }
}

void WorksheetHierarchyManager::deletePlot(const QString& commandId, const QString& resultId)
{
    if (m_worksheet->m_readOnly || commandId.isEmpty() || resultId.isEmpty())
        return;

    const CommandSearchResult commandSearch = findCommandEntryById(commandId);
    auto* commandEntry = commandSearch.entry;
    auto* expression = commandEntry ? commandEntry->expression() : nullptr;

    if (!commandEntry || !expression)
        return;

    for (auto* result : expression->results())
    {
        if (!result || result->resultId() != resultId || !isPlotResult(result))
            continue;

        const QString plotNodeId = buildPlotNodeId(commandId, resultId);
        const bool wasCurrentNode = m_currentTocNodeId == plotNodeId;

        expression->removeResult(result);

        if (wasCurrentNode)
            setCurrentTocNode(buildCommandNodeId(commandEntry));

        m_worksheet->setModified();
        scheduleTocStructureRefresh();
        return;
    }
}

void WorksheetHierarchyManager::renameHierarchyEntry(const QString& hierarchyId, const QString& newName)
{
    if (m_worksheet->m_readOnly || hierarchyId.isEmpty())
        return;

    QString normalizedName = newName;

    normalizedName.replace(QLatin1Char('\r'), QLatin1Char(' '));
    normalizedName.replace(QLatin1Char('\n'), QLatin1Char(' '));

    normalizedName = normalizedName.trimmed();

    const HierarchySearchResult hierarchySearch = findHierarchyEntryById(hierarchyId);
    auto* hierarchyEntry = hierarchySearch.entry;

    if (!hierarchyEntry || hierarchyEntry->text() == normalizedName)
        return;

    hierarchyEntry->setContent(normalizedName);
}

void WorksheetHierarchyManager::changeHierarchyLevel(const QString& hierarchyId, int levelDelta)
{
    if (m_worksheet->m_readOnly || hierarchyId.isEmpty() || (levelDelta != -1 && levelDelta != 1))
        return;

    const HierarchySearchResult hierarchySearch = findHierarchyEntryById(hierarchyId);
    HierarchyEntry* targetEntry = hierarchySearch.entry;

    if (!targetEntry)
        return;

    bool expanded = false;

    const int minimumLevel = static_cast<int>(HierarchyEntry::HierarchyLevel::Chapter);

    const int maximumLevel = static_cast<int>(HierarchyEntry::HierarchyLevel::Subparagraph);

    const int currentRootLevel = static_cast<int>(targetEntry->level());

    if (levelDelta < 0)
    {
        if (currentRootLevel <= minimumLevel)
            return;

        expanded = expandHierarchyAncestors(hierarchySearch.collapsedAncestors);
    }
    else
    {
        if (currentRootLevel >= maximumLevel)
            return;

        expanded = expandHierarchyAncestors(hierarchySearch.collapsedAncestors);

        bool hasPreviousSibling = false;

        for (auto* entry = targetEntry->previous(); entry; entry = entry->previous())
        {
            if (entry->type() != HierarchyEntry::Type)
                continue;

            const int entryLevel = static_cast<int>(static_cast<HierarchyEntry*>(entry)->level());

            if (entryLevel < currentRootLevel)
                break;

            if (entryLevel == currentRootLevel)
            {
                hasPreviousSibling = true;
                break;
            }
        }

        if (!hasPreviousSibling)
        {
            if (expanded)
            {
                updateHierarchyLayout();
                m_worksheet->updateLayout();
            }

            return;
        }
    }

    expanded = expandHierarchyForStructureChange(targetEntry) || expanded;
    const std::vector<WorksheetEntry*> subentries = hierarchySubelements(targetEntry);

    if (levelDelta > 0)
    {
        for (auto* entry : subentries)
        {
            if (!entry || entry->type() != HierarchyEntry::Type)
                continue;

            const int entryLevel = static_cast<int>(static_cast<HierarchyEntry*>(entry)->level());

            if (entryLevel >= maximumLevel)
            {
                updateHierarchyLayout();
                if (expanded)
                    m_worksheet->updateLayout();
                return;
            }
        }
    }

    const auto shiftHierarchyEntry = [levelDelta](HierarchyEntry* hierarchyEntry)
    {
        if (!hierarchyEntry)
            return;

        const int newLevel = static_cast<int>(hierarchyEntry->level()) + levelDelta;

        hierarchyEntry->setLevel(static_cast<HierarchyEntry::HierarchyLevel>(newLevel));
    };

    shiftHierarchyEntry(targetEntry);

    for (auto* entry : subentries)
    {
        if (!entry || entry->type() != HierarchyEntry::Type)
            continue;

        shiftHierarchyEntry(static_cast<HierarchyEntry*>(entry));
    }

    updateHierarchyLayout();
    m_worksheet->updateLayout();

    m_worksheet->setModified();
}

void WorksheetHierarchyManager::deleteHierarchyEntry(const QString& hierarchyId, bool deleteContents)
{
    if (m_worksheet->m_readOnly || hierarchyId.isEmpty())
        return;

    const HierarchySearchResult hierarchySearch = findHierarchyEntryById(hierarchyId);
    HierarchyEntry* targetEntry = hierarchySearch.entry;

    if (!targetEntry)
        return;

    QString headingLabel = targetEntry->text().trimmed();

    if (headingLabel.isEmpty())
        headingLabel = targetEntry->hierarchyText();

    QString warningText;
    QString dialogTitle;

    if (deleteContents)
    {
        warningText = i18n("Do you really want to delete "
                           "the heading \"%1\" and all "
                           "entries in this section? "
                           "This action cannot be undone.",
                           headingLabel);

        dialogTitle = i18n("Delete Section");
    }
    else
    {
        warningText = i18n("Do you really want to delete "
                           "only the heading \"%1\"? "
                           "The contents of the section "
                           "will remain in the worksheet. "
                           "This action cannot be undone.",
                           headingLabel);

        dialogTitle = i18n("Delete Heading");
    }

    if (Settings::warnAboutEntryDelete())
    {
        const auto result = KMessageBox::warningTwoActions(
                m_worksheet->worksheetView(),
                warningText,
                dialogTitle,
                KStandardGuiItem::remove(),
                KStandardGuiItem::cancel());

        if (result != KMessageBox::PrimaryAction)
            return;
    }

    expandHierarchyAncestors(hierarchySearch.collapsedAncestors);
    expandHierarchyForStructureChange(targetEntry);

    const std::vector<WorksheetEntry*> subentries = hierarchySubelements(targetEntry);

    m_worksheet->clearAllSelections();
    m_worksheet->notifyEntryFocus(nullptr);

    QList<WorksheetEntry*> entriesToRemove;
    entriesToRemove.append(targetEntry);

    if (deleteContents)
    {
        for (auto* entry : subentries)
            entriesToRemove.append(entry);
    }
    else
    {
        const int minimumLevel = static_cast<int>(HierarchyEntry::HierarchyLevel::Chapter);

        for (auto* entry : subentries)
        {
            if (!entry || entry->type() != HierarchyEntry::Type)
                continue;

            auto* childHeading = static_cast<HierarchyEntry*>(entry);
            const int newLevel = qMax(minimumLevel, static_cast<int>(childHeading->level()) - 1);
            childHeading->setLevel(static_cast<HierarchyEntry::HierarchyLevel>(newLevel));
        }
    }

    WorksheetEntry* previousEntry = targetEntry->previous();
    WorksheetEntry* lastRemovedEntry = entriesToRemove.constLast();
    WorksheetEntry* nextEntry = lastRemovedEntry->next();

    if (previousEntry)
        previousEntry->setNext(nextEntry);
    else
        m_worksheet->setFirstEntry(nextEntry);

    if (nextEntry)
        nextEntry->setPrevious(previousEntry);
    else
        m_worksheet->setLastEntry(previousEntry);

    m_worksheet->clearFocus();

    m_worksheet->updateFocusedTextItem(static_cast<WorksheetTextItem*>(nullptr));
    m_worksheet->updateFocusedTextItem(static_cast<WorksheetTextEditorItem*>(nullptr));

    for (auto* entry : entriesToRemove)
    {
        if (!entry)
            continue;

        entry->setPrevious(nullptr);
        entry->setNext(nullptr);
        entry->clearFocus();
        entry->hide();
        entry->deleteLater();
    }

    WorksheetEntry* focusTarget = nextEntry ? nextEntry : previousEntry;

    if (!m_worksheet->firstEntry())
        focusTarget = m_worksheet->appendCommandEntry();

    updateHierarchyLayout();
    m_worksheet->updateLayout();

    if (focusTarget)
    {
        focusTarget->focusEntry();
        m_worksheet->makeVisible(focusTarget);
        updateCurrentHierarchyFromEntry(focusTarget);
    }
    else
    {
        m_trackingSource = TrackingSource::FocusedEntry;
        updateCurrentHierarchy(nullptr);
    }

    m_worksheet->resetEntryCursor();
    m_worksheet->setModified();
}

QString WorksheetHierarchyManager::hierarchyIdForEntry(WorksheetEntry* entry) const
{
    for (auto* current = entry; current; current = current->previous())
    {
        if (current->type() == HierarchyEntry::Type)
            return static_cast<HierarchyEntry*>(current)->hierarchyId();
    }

    return QString();
}

void WorksheetHierarchyManager::setCurrentTocNode(const QString& nodeId)
{
    if (m_currentTocNodeId == nodeId)
        return;

    m_currentTocNodeId = nodeId;

    Q_EMIT m_worksheet->currentTocNodeChanged(nodeId);
}

void WorksheetHierarchyManager::updateCurrentHierarchy(WorksheetEntry* entry)
{
    QString nodeId;

    if (entry && entry->type() == CommandEntry::Type)
        nodeId = buildCommandNodeId(static_cast<CommandEntry*>(entry));
    else
        nodeId = hierarchyIdForEntry(entry);

    setCurrentTocNode(nodeId);
}

void WorksheetHierarchyManager::updateCurrentHierarchyFromView(const QRectF& viewRect)
{
    if (m_trackingSource != TrackingSource::Viewport || m_worksheet->m_layoutUpdateInProgress || m_worksheet->m_isLoadingFromFile || viewRect.isEmpty())
        return;

    const qreal activationOffset = qMin<qreal>(48.0, viewRect.height() * 0.15);

    const qreal activationY = viewRect.top() + activationOffset;

    WorksheetEntry* activeEntry = nullptr;

    for (auto* entry = m_worksheet->firstEntry(); entry; entry = entry->next())
    {
        if (!entry->isVisible())
            continue;

        if (entry->scenePos().y() > activationY)
            break;

        activeEntry = entry;
    }

    updateCurrentHierarchy(activeEntry);
}

void WorksheetHierarchyManager::updateCurrentHierarchyFromEntry(WorksheetEntry* entry)
{
    m_trackingSource = TrackingSource::FocusedEntry;

    updateCurrentHierarchy(entry);
}

void WorksheetHierarchyManager::followHierarchyFromView()
{
    m_trackingSource = TrackingSource::Viewport;

    // Defer until viewRect() reflects the final scroll position.
    QTimer::singleShot(0, this, [this]() {
        if (m_trackingSource != TrackingSource::Viewport)
            return;

        updateCurrentHierarchyFromView(m_worksheet->worksheetView()->viewRect());
    });
}

void WorksheetHierarchyManager::normalizeDraggedHierarchyLevels(HierarchyEntry* rootEntry, WorksheetEntry* previousEntry, const std::vector<WorksheetEntry*>& subentries)
{
    if (!rootEntry)
        return;

    HierarchyEntry* previousHierarchyEntry = nullptr;

    for (auto* entry = previousEntry; entry; entry = entry->previous())
    {
        if (entry->type() != HierarchyEntry::Type)
            continue;

        previousHierarchyEntry = static_cast<HierarchyEntry*>(entry);
        break;
    }

    const int minimumLevel = static_cast<int>(HierarchyEntry::HierarchyLevel::Chapter);
    const int maximumLevel = static_cast<int>(HierarchyEntry::HierarchyLevel::Subparagraph);

    int maximumAllowedRootLevel = minimumLevel;

    if (previousHierarchyEntry)
        maximumAllowedRootLevel = qMin(maximumLevel, static_cast<int>(previousHierarchyEntry->level()) + 1);

    const int oldRootLevel = static_cast<int>(rootEntry->level());
    if (oldRootLevel <= maximumAllowedRootLevel)
        return;

    const int levelOffset = maximumAllowedRootLevel - oldRootLevel;

    const auto shiftLevel = [levelOffset, minimumLevel, maximumLevel](HierarchyEntry* hierarchyEntry)
    {
        if (!hierarchyEntry)
            return;

        const int newLevel = qBound(minimumLevel, static_cast<int>(hierarchyEntry->level()) + levelOffset, maximumLevel);

        hierarchyEntry->setLevel(static_cast<HierarchyEntry::HierarchyLevel>(newLevel));
    };

    shiftLevel(rootEntry);

    for (auto* entry : subentries)
    {
        if (entry->type() != HierarchyEntry::Type)
            continue;

        shiftLevel(static_cast<HierarchyEntry*>(entry));
    }
}
