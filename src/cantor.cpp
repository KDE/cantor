/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/
#include "cantor.h"
#include "lib/session.h"

#include <cassert>

#include <KActionCollection>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KConfig>
#include <KMessageBox>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KNS3/DownloadDialog>
#include <KParts/ReadWritePart>
#include <KRecentFilesAction>

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QStatusBar>
#include <QGraphicsView>
#include <QPushButton>
#include <QRegularExpression>


#include "lib/backend.h"
#include "lib/panelpluginhandler.h"
#include "lib/panelplugin.h"
#include "lib/worksheetaccess.h"

#include "backendchoosedialog.h"
#include "settings.h"
#include "ui_settings.h"
#include "ui_formating.h"
#include "backendchoosedialog.h"
#include <QMetaObject>

CantorShell::CantorShell() : KParts::MainWindow(), m_tabWidget(new QTabWidget(this))
{
    // set the shell's ui resource file
    setXMLFile(QLatin1String("cantor_shell.rc"));

    // then, setup our actions
    setupActions();

    createGUI(nullptr);

    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &CantorShell::activateWorksheet);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &CantorShell::closeTab);

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowTabbedDocks|QMainWindow::VerticalTabs);

    initPanels();

    updateNewSubmenu();
}

CantorShell::~CantorShell()
{
    if (m_recentProjectsAction)
        m_recentProjectsAction->saveEntries(KSharedConfig::openConfig()->group(QLatin1String("Recent Files")));

    if (!m_newBackendActions.isEmpty())
    {
        unplugActionList(QLatin1String("new_worksheet_with_backend_list"));
        qDeleteAll(m_newBackendActions);
        m_newBackendActions.clear();
    }

    unplugActionList(QLatin1String("view_show_panel_list"));
    qDeleteAll(m_panels);
    m_panels.clear();
}

void CantorShell::load(const QUrl& url)
{
    // If the url already opened, then don't open the url in another tab, but
    // just activate the already existed tab
    for (int i = 0; i < m_parts.size(); i++)
    {
        auto* part = m_parts[i];
        if (part && part->url() == url)
        {
            if (m_tabWidget->currentIndex() != i)
                activateWorksheet(i);
            else
                KMessageBox::information(this, i18n("The file %1 is already opened.", QFileInfo(url.toLocalFile()).fileName()), i18n("Open file"));
            return;
        }
    }

    if (!m_part || !m_part->url().isEmpty() || m_part->isModified())
    {
        addWorksheet(QString());
        m_tabWidget->setCurrentIndex(m_parts.size() - 1);
    }

    if (!m_part->openUrl(url))
        closeTab(m_tabWidget->currentIndex());

    if (m_recentProjectsAction)
        m_recentProjectsAction->addUrl(url);

    updateWindowTitle(m_part->url().fileName());
}

void CantorShell::setupActions()
{
    QAction* openNew = KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    openNew->setPriority(QAction::LowPriority);
    QAction* open = KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    open->setPriority(QAction::LowPriority);
    m_recentProjectsAction = KStandardAction::openRecent(this, &CantorShell::load, actionCollection());
    m_recentProjectsAction->setPriority(QAction::LowPriority);
    m_recentProjectsAction->loadEntries(KSharedConfig::openConfig()->group(QLatin1String("Recent Files")));

    KStandardAction::close(this, SLOT(closeTab()),  actionCollection());

    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());

    createStandardStatusBarAction();
    //setStandardToolBarMenuEnabled(true);

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());

    QAction* downloadExamples = new QAction(i18n("Download Examples"), actionCollection());
    downloadExamples->setIcon(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")));
    actionCollection()->addAction(QLatin1String("file_example_download"),  downloadExamples);
    connect(downloadExamples, &QAction::triggered, this,  &CantorShell::downloadExamples);

    QAction* openExample = new QAction(i18n("&Open Example"), actionCollection());
    openExample->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    actionCollection()->addAction(QLatin1String("file_example_open"), openExample);
    connect(openExample, &QAction::triggered, this, &CantorShell::openExample);

    QAction* toPreviousTab = new QAction(i18n("Go to previous worksheet"), actionCollection());
    actionCollection()->addAction(QLatin1String("go_to_previous_tab"), toPreviousTab);
    actionCollection()->setDefaultShortcut(toPreviousTab, Qt::CTRL+Qt::Key_PageDown);
    connect(toPreviousTab, &QAction::triggered, toPreviousTab, [this](){
        const int index = m_tabWidget->currentIndex()-1;
        if (index >= 0)
            m_tabWidget->setCurrentIndex(index);
        else
            m_tabWidget->setCurrentIndex(m_tabWidget->count()-1);
    });
    addAction(toPreviousTab);

    QAction* toNextTab = new QAction(i18n("Go to next worksheet"), actionCollection());
    actionCollection()->addAction(QLatin1String("go_to_next_tab"), toNextTab);
    actionCollection()->setDefaultShortcut(toNextTab, Qt::CTRL+Qt::Key_PageUp);
    connect(toNextTab, &QAction::triggered, toNextTab, [this](){
        const int index = m_tabWidget->currentIndex()+1;
        if (index < m_tabWidget->count())
            m_tabWidget->setCurrentIndex(index);
        else
            m_tabWidget->setCurrentIndex(0);
    });
    addAction(toNextTab);
}

void CantorShell::saveProperties(KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void CantorShell::readProperties(const KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

/*!
 * called when one of the "new backend" action or the "New" action are called
 * adds a new worksheet with the backend assossiated with the called action
 * or opens the "Choose Backend" dialog, respectively.
 */
void CantorShell::fileNew()
{
    QAction* a = static_cast<QAction*>(sender());
    const QString& backendName = a->data().toString();
    if (!backendName.isEmpty())
    {
        addWorksheet(backendName);
        return;
    }

    //"New" action was called -> open the "Choose Backend" dialog.
    addWorksheet();
}

void CantorShell::optionsConfigureKeys()
{
    KShortcutsDialog dlg( KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, this );
    dlg.addCollection( actionCollection(), i18n("Cantor") );
    if (m_part)
        dlg.addCollection( m_part->actionCollection(), i18n("Cantor") );
    dlg.configure( true );
}

void CantorShell::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    static const QString& filter = i18n("All supported files (*.cws *ipynb);;Cantor Worksheet (*.cws);;Jupyter Notebook (*.ipynb)");
    const QUrl& url = QFileDialog::getOpenFileUrl(this, i18n("Open file"), QUrl(), filter, &m_previousFilter);

    if (url.isEmpty() == false)
    {
        // About this function, the style guide
        // says that it should open a new window if the document is _not_
        // in its initial state.  This is what we do here..
        /*if ( m_part->url().isEmpty() && ! m_part->isModified() )
        {
            // we open the file in this window...
            load( url );
        }
        else
        {
            // we open the file in a new window...
            CantorShell* newWin = new CantorShell;
            newWin->load( url );
            newWin->show();
            }*/
        load(url);
    }
}

void CantorShell::addWorksheet()
{
    bool hasBackend = false;
    for (auto* b : Cantor::Backend::availableBackends())
    {
        if(b->isEnabled())
        {
            hasBackend = true;
            break;
        }
    }

    if(hasBackend) //There is no point in asking for the backend, if no one is available
    {
        QString backend = Settings::self()->defaultBackend();
        if (backend.isEmpty())
        {
            QPointer<BackendChooseDialog> dlg=new BackendChooseDialog(this);
            if(dlg->exec())
            {
                backend = dlg->backendName();
                addWorksheet(backend);
            }

            delete dlg;
        }
        else
        {
            addWorksheet(backend);
        }

    }else
    {
        QTextBrowser* browser = new QTextBrowser(this);
        QString backendList = QLatin1String("<ul>");
        int backendListSize = 0;
        for (auto* backend : Cantor::Backend::availableBackends())
        {
            if(!backend->requirementsFullfilled()) //It's disabled because of missing dependencies, not because of some other reason(like eg. nullbackend)
            {
                backendList += QString::fromLatin1("<li>%1: <a href=\"%2\">%2</a></li>").arg(backend->name(), backend->url());
                ++backendListSize;
            }
        }
        browser->setHtml(i18np("<h1>No Backend Found</h1>\n"             \
                               "<div>You could try:\n"                   \
                               "  <ul>"                                  \
                               "    <li>Changing the settings in the config dialog;</li>" \
                               "    <li>Installing packages for the following program:</li>" \
                               "     %2 "                                \
                               "  </ul> "                                 \
                               "</div> "
                              , "<h1>No Backend Found</h1>\n"             \
                               "<div>You could try:\n"                   \
                               "  <ul>"                                  \
                               "    <li>Changing the settings in the config dialog;</li>" \
                               "    <li>Installing packages for one of the following programs:</li>" \
                               "     %2 "                                \
                               "  </ul> "                                 \
                               "</div> "
                              , backendListSize, backendList
                              ));

        browser->setObjectName(QLatin1String("ErrorMessage"));
        m_tabWidget->addTab(browser, i18n("Error"));
    }
}

void CantorShell::addWorksheet(const QString& backendName)
{
    static int sessionCount=1;

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KPluginLoader loader(QLatin1String("kf5/parts/cantorpart"));
    KPluginFactory* factory = loader.factory();
    if (factory)
    {
        Cantor::Backend* backend = nullptr;
        if (!backendName.isEmpty())
        {
            backend = Cantor::Backend::getBackend(backendName);
            if (!backend)
            {
                KMessageBox::error(this, i18n("Backend %1 is not installed", backendName), i18n("Cantor"));
                return;
            }
            else
            {
                if (!backend->isEnabled())
                {
                    KMessageBox::error(this, i18n("%1 backend installed, but inactive. Please check installation and Cantor settings", backendName), i18n("Cantor"));
                    return;
                }
            }
        }

        // now that the Part is loaded, we cast it to a Part to get our hands on it
        auto* part = factory->create<KParts::ReadWritePart>(m_tabWidget, QVariantList()<<backendName);
        if (part)
        {
            connect(part, SIGNAL(setCaption(QString,QIcon)), this, SLOT(setTabCaption(QString,QIcon)));
            connect(part, SIGNAL(worksheetSave(QUrl)), this, SLOT(onWorksheetSave(QUrl)));
            connect(part, SIGNAL(showHelp(QString)), this, SIGNAL(showHelp(QString)));
            connect(part, SIGNAL(hierarchyChanged(QStringList, QStringList, QList<int>)), this, SIGNAL(hierarchyChanged(QStringList, QStringList, QList<int>)));
            connect(part, SIGNAL(hierarhyEntryNameChange(QString, QString, int)), this, SIGNAL(hierarhyEntryNameChange(QString, QString, int)));
            connect(this, SIGNAL(requestScrollToHierarchyEntry(QString)), part, SIGNAL(requestScrollToHierarchyEntry(QString)));
            connect(this, SIGNAL(settingsChanges()), part, SIGNAL(settingsChanges()));
            connect(part, SIGNAL(requestDocumentation(QString)), this, SIGNAL(requestDocumentation(QString)));

            m_parts.append(part);
            if (backend) // If backend empty (loading worksheet from file), then we connect to signal and wait
                m_parts2Backends[part] = backend->id();
            else
            {
                m_parts2Backends[part] = QString();
                connect(part, SIGNAL(setBackendName(QString)), this, SLOT(updateBackendForPart(QString)));
            }
            int tab = m_tabWidget->addTab(part->widget(), i18n("Session %1", sessionCount++));
            m_tabWidget->setCurrentIndex(tab);
            // Setting focus on worksheet view, because Qt clear focus of added widget inside addTab
            // This fix https://bugs.kde.org/show_bug.cgi?id=395976
            part->widget()->findChild<QGraphicsView*>()->setFocus();

            // Force run updateCaption for getting proper backend icon
            QMetaObject::invokeMethod(part, "updateCaption");
        }
        else
        {
            qDebug()<<"error creating part ";
        }
    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Failed to find the Cantor Part with error %1", loader.errorString()));
        qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

}

void CantorShell::activateWorksheet(int index)
{
    // Save part panels states before change worksheet
    if (m_part)
    {
        QStringList visiblePanelNames;
        for (auto* doc : m_panels)
        {
            if (doc->widget() && doc->widget()->isVisible())
                visiblePanelNames << doc->objectName();
        }
        m_pluginsVisibility[m_part] = visiblePanelNames;

        auto* wa = m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);
        assert(wa);
        Cantor::PanelPluginHandler::PanelStates states;
        auto plugins = m_panelHandler.plugins(wa->session());
        for(auto* plugin : plugins)
        {
            states.insert(plugin->name(), plugin->saveState());
        }
        m_pluginsStates[m_part] = states;
    }

    if (index != -1)
    {
        m_part = findPart(m_tabWidget->widget(index));
        if(m_part)
        {
            createGUI(m_part);

            //update the status bar
            auto* wa = m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);
            if (wa->session())
            {
                auto status = wa->session()->status();
                switch (status) {
                    case Cantor::Session::Running:
                        statusBar()->showMessage(i18n("Calculating..."));
                        break;
                    case Cantor::Session::Done:
                        statusBar()->showMessage(i18n("Ready"));
                        break;
                    case Cantor::Session::Disable:
                        statusBar()->showMessage(QString());
                        break;
                }
            }

            updateWindowTitle(m_part->url().fileName());
            updatePanel();
        }
        else
            qDebug()<<"selected part doesn't exist";

        m_tabWidget->setCurrentIndex(index);
    }
}

void CantorShell::setTabCaption(const QString& caption, const QIcon& icon)
{
    auto* part = dynamic_cast<KParts::ReadWritePart*>(sender());
    if (part)
    {
        if (!caption.isEmpty())
            m_tabWidget->setTabText(m_tabWidget->indexOf(part->widget()), caption);
        m_tabWidget->setTabIcon(m_tabWidget->indexOf(part->widget()), icon);
    }
}

void CantorShell::updateWindowTitle(const QString& fileName)
{
    QFileInfo info(fileName);
    setWindowTitle(info.baseName());
}

void CantorShell::closeTab(int index)
{
    if (index != -1)
    {
        QWidget* widget = m_tabWidget->widget(index);
        if (widget)
        {
            auto* part = findPart(widget);
            if (part && !reallyCloseThisPart(part))
                return;
        }
    }
    else
    {
        if (!reallyClose(false))
            return;
    }

    QWidget* widget = nullptr;
    if (index >= 0)
    {
        widget = m_tabWidget->widget(index);
    }
    else if (m_part)
    {
        widget = m_part->widget();
    }

    if (!widget)
    {
        qWarning() << "Could not find widget by tab index" << index;
        return;
    }


    m_tabWidget->removeTab(index);

    bool isCurrectPartClosed = m_part ? widget == m_part->widget() : false;
    if(widget->objectName() == QLatin1String("ErrorMessage"))
    {
        widget->deleteLater();
    }else
    {
        auto* part = findPart(widget);
        if(part)
        {
            saveDockPanelsState(part);

            m_parts.removeAll(part);
            m_pluginsVisibility.remove(part);
            m_parts2Backends.remove(part);
            m_pluginsStates.remove(part);
            delete part;
        }
    }

    if (m_tabWidget->count() == 0)
        setCaption(QString());

    if (isCurrectPartClosed || m_part == nullptr)
        updatePanel();
}

bool CantorShell::reallyClose(bool checkAllParts) {
    if(checkAllParts && m_parts.count() > 1) {
        bool modified = false;
        for (auto* part : m_parts)
        {
            if(part->isModified()) {
                modified = true;
                break;
            }
        }
        if(!modified) return true;
        int want_save = KMessageBox::warningYesNo( this,
            i18n("Multiple unsaved Worksheets are opened. Do you want to close them?"),
            i18n("Close Cantor"));
        switch (want_save) {
            case KMessageBox::Yes:
                return true;
            case KMessageBox::No:
                return false;
        }
    }

    return reallyCloseThisPart(m_part);
}

bool CantorShell::reallyCloseThisPart(KParts::ReadWritePart* part)
{
     if (part && part->isModified() ) {
        int want_save = KMessageBox::warningYesNoCancel( this,
            i18n("The current project has been modified. Do you want to save it?"),
            i18n("Save Project"));
        switch (want_save) {
            case KMessageBox::Yes:
                part->save();
                if(part->waitSaveComplete()) {
                    return true;
                } else {
                    part->setModified(true);
                    return false;
                }
            case KMessageBox::Cancel:
                return false;
            case KMessageBox::No:
                return true;
        }
    }
    return true;
}


void CantorShell::closeEvent(QCloseEvent* event) {
    if (!reallyClose()) {
        event->ignore();
    } else {
        for (auto* part : m_parts)
            saveDockPanelsState(part);

        KParts::MainWindow::closeEvent(event);
    }
}

void CantorShell::showSettings()
{
    KConfigDialog *dialog = new KConfigDialog(this,  QLatin1String("settings"), Settings::self());

    QWidget *generalSettings = new QWidget;
    Ui::SettingsBase base;
    base.setupUi(generalSettings);

    QWidget *formattingSettings = new QWidget;
    Ui::SettingsFormatting formatting;
    formatting.setupUi(formattingSettings);

    base.kcfg_DefaultBackend->addItems(Cantor::Backend::listAvailableBackends());

    dialog->addPage(generalSettings, i18n("General"), QLatin1String("preferences-other"));
    dialog->addPage(formattingSettings, i18n("Formatting"), QLatin1String("preferences-other"));
    for (auto* backend : Cantor::Backend::availableBackends())
    {
        if (backend->config()) //It has something to configure, so add it to the dialog
            dialog->addPage(backend->settingsWidget(dialog), backend->config(), backend->name(),  backend->icon());
    }

    dialog->show();
    connect(dialog, &KConfigDialog::settingsChanged, this, &CantorShell::settingsChanges);
}

void CantorShell::downloadExamples()
{
    KNS3::DownloadDialog dialog;
    dialog.exec();
    for (const auto& e :  dialog.changedEntries())
    {
        qDebug() << "Changed Entry: " << e.name();
    }
}

void CantorShell::openExample()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/examples");
    if (dir.isEmpty())
        return;

    QDir().mkpath(dir);

    QStringList files = QDir(dir).entryList(QDir::Files);
    QPointer<QDialog> dlg = new QDialog(this);
    QListWidget* list = new QListWidget(dlg);
    for (const QString& file : files)
    {
        QString name = file;
        name.remove(QRegularExpression(QStringLiteral("-.*\\.hotstuff-access$")));
        list->addItem(name);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(list);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    connect(buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    if (dlg->exec() == QDialog::Accepted&&list->currentRow()>=0)
    {
        const QString& selectedFile = files[list->currentRow()];
        const QUrl& url = QUrl::fromLocalFile(QDir(dir).absoluteFilePath(selectedFile));

        qDebug()<<"loading file "<<url;
        load(url);
    }

    delete dlg;
}

KParts::ReadWritePart* CantorShell::findPart(QWidget* widget)
{
    for (auto* part : m_parts)
    {
        if (part->widget() == widget)
            return part;
    }
    return nullptr;
}

void CantorShell::initPanels()
{
    m_panelHandler.loadPlugins();

    for (auto* plugin : m_panelHandler.allPlugins())
    {
        if (!plugin)
        {
            qDebug()<<"invalid panel found, skipping it.";
            continue;
        }

        qDebug()<<"adding panel for "<<plugin->name();
        plugin->setParentWidget(this);
        plugin->connectToShell(this);

        QDockWidget* docker = new QDockWidget(plugin->name(), this);
        docker->setObjectName(plugin->name());
        docker->setWidget(plugin->widget());
        docker->setWindowIcon(QIcon::fromTheme(QStringLiteral("format-text-bold")));
        addDockWidget(Qt::RightDockWidgetArea, docker);

        docker->hide();

        connect(plugin, &Cantor::PanelPlugin::visibilityRequested, this, &CantorShell::pluginVisibilityRequested);
        connect(plugin, &Cantor::PanelPlugin::requestRunCommand, this, &CantorShell::pluginCommandRunRequested);

        m_panels.append(docker);
    }
}

void CantorShell::updatePanel()
{
    unplugActionList(QLatin1String("view_show_panel_list"));

    QList<QAction*> panelActions;

    bool isNewWorksheet = !m_pluginsVisibility.contains(m_part);
    if (isNewWorksheet)
    {
        KConfigGroup panelStatusGroup(KSharedConfig::openConfig(), QLatin1String("PanelsStatus"));
        if (m_parts2Backends.contains(m_part) && panelStatusGroup.hasKey(m_parts2Backends[m_part]))
        {
            const QStringList& plugins = panelStatusGroup.readEntry(m_parts2Backends[m_part]).split(QLatin1Char('\n'));
            m_pluginsVisibility[m_part] = plugins;
            isNewWorksheet = false;
        }
    }

    Cantor::WorksheetAccessInterface* wa = nullptr;
    if (m_part)
        wa = m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);

    // Worksheet interface can be missing on m_part clossing (and m_part on this moment can be nullptr)
    QList<Cantor::PanelPlugin*> plugins;
    if (wa)
    {
        QDockWidget* last = nullptr;
        plugins = m_panelHandler.activePluginsForSession(wa->session(), m_pluginsStates.contains(m_part) ? m_pluginsStates[m_part] : Cantor::PanelPluginHandler::PanelStates());
        for (auto* plugin : plugins)
        {
            QDockWidget* foundDocker = nullptr;
            for (auto* docker : m_panels)
                if (docker->objectName() == plugin->name())
                {
                    foundDocker = docker;
                    break;
                }

            if (!foundDocker)
            {
                qDebug() << "something wrong: can't find panel for plugin \"" << plugin->name() << "\"";
                continue;
            }

            // Set visibility for dock from saved info
            if (isNewWorksheet)
            {
                if (plugin->showOnStartup())
                    foundDocker->show();
                else
                    foundDocker->hide();
            }
            else
            {
                if (m_pluginsVisibility[m_part].contains(plugin->name()))
                    foundDocker->show();
                else
                    foundDocker->hide();
            }

            if(last!=nullptr)
                tabifyDockWidget(last, foundDocker);
            last = foundDocker;

            //Create the action to show/hide this panel
            panelActions<<foundDocker->toggleViewAction();

        }
    }

    // Hide plugins, which don't supported on current session
    QList<Cantor::PanelPlugin*> allPlugins=m_panelHandler.allPlugins();
    for(auto* plugin : allPlugins)
    {
        if (plugins.indexOf(plugin) == -1)
            for (QDockWidget* docker : m_panels)
                if (docker->objectName() == plugin->name())
                {
                    docker->hide();
                    break;
                }
    }

    plugActionList(QLatin1String("view_show_panel_list"), panelActions);

    updateNewSubmenu();
}

void CantorShell::updateNewSubmenu()
{
    unplugActionList(QLatin1String("new_worksheet_with_backend_list"));
    qDeleteAll(m_newBackendActions);
    m_newBackendActions.clear();

    for (auto* backend : Cantor::Backend::availableBackends())
    {
        if (!backend->isEnabled())
            continue;
        QAction* action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(), nullptr);
        action->setData(backend->name());
        connect(action, SIGNAL(triggered()), this, SLOT(fileNew()));
        m_newBackendActions << action;
    }
    plugActionList(QLatin1String("new_worksheet_with_backend_list"), m_newBackendActions);
}

Cantor::WorksheetAccessInterface* CantorShell::currentWorksheetAccessInterface()
{
    auto* wa = m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);
    return wa;
}

void CantorShell::pluginVisibilityRequested()
{
    auto* plugin = static_cast<Cantor::PanelPlugin*>(sender());
    for (auto* docker : m_panels)
    {
        if (plugin->name() == docker->objectName())
        {
            if (docker->isHidden())
                docker->show();
            docker->raise();
        }
    }
}

void CantorShell::onWorksheetSave(const QUrl& url)
{
    if (m_recentProjectsAction)
        m_recentProjectsAction->addUrl(url);

    updateWindowTitle(m_part->url().fileName());
}

void CantorShell::saveDockPanelsState(KParts::ReadWritePart* part)
{
    if (m_parts2Backends.contains(part))
    {
        QStringList visiblePanelNames;
        if (part == m_part)
        {
            for (auto* doc : m_panels)
            {
                if (doc->widget() && doc->widget()->isVisible())
                    visiblePanelNames << doc->objectName();
            }
        }
        else if (m_pluginsVisibility.contains(part))
            visiblePanelNames = m_pluginsVisibility[part];

        KConfigGroup panelStatusGroup(KSharedConfig::openConfig(), QLatin1String("PanelsStatus"));
        panelStatusGroup.writeEntry(m_parts2Backends[part], visiblePanelNames.join(QLatin1Char('\n')));
    }
}

void CantorShell::updateBackendForPart(const QString& backend)
{
    auto* part = dynamic_cast<KParts::ReadWritePart*>(sender());
    if (part && m_parts2Backends.contains(part) && m_parts2Backends[part].isEmpty())
    {
        m_parts2Backends[part] = backend;

        KConfigGroup panelStatusGroup(KSharedConfig::openConfig(), QLatin1String("PanelsStatus"));
        if (m_part == part && panelStatusGroup.hasKey(backend))
        {
            const QStringList& plugins = panelStatusGroup.readEntry(m_parts2Backends[m_part]).split(QLatin1Char('\n'));
            m_pluginsVisibility[m_part] = plugins;

            updatePanel();
        }
    }
}

void CantorShell::pluginCommandRunRequested(const QString& cmd)
{
    if (m_part)
        QMetaObject::invokeMethod(m_part, "runCommand", Qt::QueuedConnection, Q_ARG(QString, cmd));
}
