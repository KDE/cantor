/*
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef CANTOR_H
#define CANTOR_H

#include <QObject>
#include <KParts/MainWindow>

#include <QList>
#include <QStringList>
#include <QMap>

#include "lib/panelpluginhandler.h"
#include "lib/panelplugin.h"

class QTabWidget;
class KRecentFilesAction;

namespace Cantor{
class WorksheetAccessInterface;
}

namespace KParts{
    class ReadWritePart;
}

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 */
class CantorShell : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    CantorShell();

    /**
     * Default Destructor
     */
    ~CantorShell() override;

    Cantor::WorksheetAccessInterface* currentWorksheetAccessInterface();
    void addWorksheet();

protected:
    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfigGroup &) override;

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(const KConfigGroup &) override;

Q_SIGNALS:
    void showHelp(QString);
    void hierarchyChanged(QStringList, QStringList, QList<int>);
    void hierarhyEntryNameChange(QString, QString, int);
    void requestScrollToHierarchyEntry(QString);
    void settingsChanges();
    void requestDocumentation(const QString&);

public Q_SLOTS:
    void addWorksheet(const QString&);
    /// Use this method/slot to load whatever file/URL you have
    void load(const QUrl&);

private Q_SLOTS:
    void fileNew();
    void fileOpen();
    void onWorksheetSave(const QUrl&);
    void optionsConfigureKeys();

    void activateWorksheet(int index);

    void setTabCaption(const QString&, const QIcon&);
    void updateBackendForPart(const QString&);
    void closeTab(int index = -1);

    void showSettings();

    void downloadExamples();
    void openExample();

    void initPanels();
    void updatePanel();
    void updateNewSubmenu();

    void pluginVisibilityRequested();
    void pluginCommandRunRequested(const QString&);

private:
    void setupActions();
    void closeEvent(QCloseEvent*) override;
    bool reallyClose(bool checkAllParts = true);
    bool reallyCloseThisPart(KParts::ReadWritePart*);
    void updateWindowTitle(const QString&);
    void saveDockPanelsState(KParts::ReadWritePart*);
    KParts::ReadWritePart* findPart(QWidget*);

private:
    QMap<KParts::ReadWritePart*, QStringList> m_pluginsVisibility;
    QMap<KParts::ReadWritePart*, Cantor::PanelPluginHandler::PanelStates> m_pluginsStates;
    QList<KParts::ReadWritePart*> m_parts;
    QMap<KParts::ReadWritePart*, QString> m_parts2Backends;
    KParts::ReadWritePart* m_part{nullptr};
    QTabWidget* m_tabWidget;
    QList<QDockWidget*> m_panels;
    QList<QAction*> m_newBackendActions;
    KRecentFilesAction* m_recentProjectsAction;

    Cantor::PanelPluginHandler m_panelHandler{nullptr};

    // For better UX: set previous used filter in "Open" action as default filter
    QString m_previousFilter;
};

#endif // CANTOR_H
