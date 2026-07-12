#ifndef WORKSHEETHIERARCHYMANAGER_H
#define WORKSHEETHIERARCHYMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

#include <functional>
#include <vector>

class QRectF;
class Worksheet;
class CommandEntry;
class WorksheetEntry;
class HierarchyEntry;

namespace Cantor {
    class Result;
}

class WorksheetHierarchyManager : public QObject
{
  public:
    explicit WorksheetHierarchyManager(Worksheet* worksheet);

    size_t hierarchyMaxDepth() const;

    void refreshTocStructure();
    void scheduleTocStructureRefresh();
    void emitTocNodeSnapshot();

    void updateHierarchyLayout();
    void updateHierarchyControlsLayout(WorksheetEntry* startEntry = nullptr);

    std::vector<WorksheetEntry*> hierarchySubelements(HierarchyEntry*) const;
    WorksheetEntry* cutSubentriesForHierarchy(HierarchyEntry*);
    void insertSubentriesForHierarchy(HierarchyEntry*, WorksheetEntry*);
    bool expandHierarchyForStructureChange(HierarchyEntry*);

    void renameHierarchyEntry(const QString& hierarchyId, const QString& newName);
    void changeHierarchyLevel(const QString& hierarchyId, int levelDelta);
    void deleteHierarchyEntry(const QString& hierarchyId, bool deleteContents);
    void renameCommandEntry(const QString& commandId, const QString& newTitle);
    void deleteCommandEntry(const QString& commandId);
    void renamePlot(const QString& commandId, const QString& resultId, const QString& newTitle);
    void deletePlot(const QString& commandId, const QString& resultId);
    void navigateToTocNode(QString nodeId);
    void updateCurrentTocNodeFromResult(CommandEntry* commandEntry, Cantor::Result* result);

    void updateCurrentHierarchy(WorksheetEntry* entry);
    void updateCurrentHierarchyFromView(const QRectF& viewRect);
    void updateCurrentHierarchyFromEntry(WorksheetEntry* entry);
    void followHierarchyFromView();

    void normalizeDraggedHierarchyLevels(HierarchyEntry* rootEntry, WorksheetEntry* previousEntry, const std::vector<WorksheetEntry*>& subentries);

  private:
    enum class TrackingSource
    {
        Viewport,
        FocusedEntry
    };

    struct HierarchySearchResult
    {
        HierarchyEntry* entry{nullptr};
        QVector<HierarchyEntry*> collapsedAncestors;
    };

    struct CommandSearchResult
    {
        CommandEntry* entry{nullptr};
        QVector<HierarchyEntry*> collapsedAncestors;
    };

    QVariantList collectTocNodes() const;
    bool visitLogicalEntries(WorksheetEntry* first, const std::function<bool(WorksheetEntry*)>& visitor) const;
    bool visitLogicalEntries(const std::function<bool(WorksheetEntry*)>& visitor) const;
    bool findHierarchyEntryById(WorksheetEntry* first, const QString& hierarchyId, QVector<HierarchyEntry*> collapsedAncestors, HierarchySearchResult& result) const;
    HierarchySearchResult findHierarchyEntryById(const QString& hierarchyId) const;
    bool findCommandEntryById(WorksheetEntry* first, const QString& commandId, QVector<HierarchyEntry*> collapsedAncestors, CommandSearchResult& result) const;
    CommandSearchResult findCommandEntryById(const QString& commandId) const;
    bool expandHierarchyAncestors(const QVector<HierarchyEntry*>& ancestors);
    QString buildCommandNodeId(CommandEntry* entry) const;
    QString buildPlotNodeId(const QString& commandId, const QString& resultId) const;
    bool parseCommandNodeId(const QString& nodeId, QString* commandId) const;
    bool parsePlotNodeId(const QString& nodeId, QString* commandId, QString* resultId) const;
    bool navigateToPlotResult(CommandEntry* commandEntry, const QString& resultId);
    void setCurrentTocNode(const QString& nodeId);
    QString hierarchyIdForEntry(WorksheetEntry* entry) const;
    QString commandTocTitle(CommandEntry* entry) const;
    QString commandTocDisplayText(CommandEntry* entry) const;
    QString plotTocTitle(Cantor::Result* result) const;
    QString plotTocDisplayText(CommandEntry* entry, Cantor::Result* result, int plotOrdinal, int plotCount) const;
    bool isPlotResult(Cantor::Result* result) const;

    Worksheet* m_worksheet;
    QString m_currentTocNodeId;
    TrackingSource m_trackingSource{TrackingSource::Viewport};
    size_t m_hierarchyMaxDepth{0};
    QVariantList m_tocNodeSnapshot;
    bool m_tocRefreshScheduled{false};
};

#endif // WORKSHEETHIERARCHYMANAGER_H
