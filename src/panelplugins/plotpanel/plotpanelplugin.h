#ifndef PLOTPANELPLUGIN_H
#define PLOTPANELPLUGIN_H

#include "panelplugin.h"

#include <QHash>
#include <QImage>
#include <QList>
#include <QPointer>
#include <QString>
#include <QVariantList>

class QAction;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QPoint;
class QStackedLayout;
class QTimer;
class QToolBar;
class QWidget;

class PlotPanelPlugin : public Cantor::PanelPlugin
{
    Q_OBJECT

public:
    PlotPanelPlugin(QObject* parent, const QList<QVariant>& args);
    ~PlotPanelPlugin() override;

    QWidget* widget() override;
    bool showOnStartup() override;
    void connectToShell(QObject* cantorShell) override;

    State saveState() override;
    void restoreState(const State& state) override;

Q_SIGNALS:
    void requestNavigateToTocNode(QString nodeId);
    void requestRenamePlot(QString commandId, QString resultId, QString newTitle);
    void requestDeletePlot(QString commandId, QString resultId);
    void requestSavePlot(QString commandId, QString resultId);
    void requestSaveAllPlots();
    void requestCopyPlot(QString commandId, QString resultId);

private Q_SLOTS:
    void handleTocNodesChanged(const QVariantList& nodes);
    void handleCurrentTocNodeChanged(const QString& nodeId);
    void handleAnimationFrameChanged(const QString& resultId, const QImage& frame);
    void handleReadOnlyChanged(bool readOnly);
    void flushAnimationFrames();
    void updateFilter(const QString& text);
    void updateCurrentPlot();
    void showContextMenu(const QPoint& position);
    void activatePlot(QListWidgetItem* item);
    void selectPreviousPlot();
    void selectNextPlot();
    void renameCurrentPlot();
    void deleteCurrentPlot();
    void saveCurrentPlot();
    void copyCurrentPlot();
    void updateAnimationMode(bool enabled);
    void updateSourceLabelMode(bool enabled);

private:
    enum ItemRole
    {
        NodeIdRole = Qt::UserRole + 1,
        EntryIdRole,
        ResultIdRole,
        CustomTitleRole,
        DisplayTextRole,
        PreviewCacheKeyRole,
        AnimatedRole,
        HasLiveFrameRole,
        FormatRole,
        SourceTextRole
    };

    void constructWidget();
    void clearPlots();
    void applyPlotNodeChanges(const QVariantList& nodes);
    void updateEmptyState();
    void updateSelection();
    void updateActionState();
    void updateItemLabels();
    void selectRelativePlot(int direction);
    QListWidgetItem* currentPlotItem() const;
    QListWidgetItem* firstVisibleItem() const;

    QPointer<QWidget> m_containerWidget;
    QPointer<QListWidget> m_plotList;
    QPointer<QLabel> m_emptyLabel;
    QPointer<QLineEdit> m_searchEdit;
    QPointer<QToolBar> m_toolbar;
    QStackedLayout* m_layout{nullptr};
    QTimer* m_animationRefreshTimer{nullptr};
    QAction* m_previousAction{nullptr};
    QAction* m_nextAction{nullptr};
    QAction* m_sourceLabelsAction{nullptr};
    QAction* m_copyAction{nullptr};
    QAction* m_saveAction{nullptr};
    QAction* m_saveAllAction{nullptr};
    QAction* m_renameAction{nullptr};
    QAction* m_deleteAction{nullptr};
    QAction* m_animationAction{nullptr};
    QHash<QString, QListWidgetItem*> m_itemsByNodeId;
    QHash<QString, QListWidgetItem*> m_itemsByResultId;
    QHash<QString, QImage> m_pendingAnimationFrames;
    QString m_currentNodeId;
    QString m_searchText;
    bool m_readOnly{false};
    bool m_animatePreviews{true};
    bool m_showSourceLabels{false};
};

#endif
