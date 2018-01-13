/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */
#include "cantor.h"

#include <KActionCollection>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KMessageBox>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KNS3/DownloadDialog>
#include <KParts/ReadWritePart>

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDockWidget>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>

#include "lib/backend.h"
#include "lib/panelpluginhandler.h"
#include "lib/panelplugin.h"
#include "lib/worksheetaccess.h"

#include "settings.h"
#include "ui_settings.h"
#include "backendchoosedialog.h"

CantorShell::CantorShell()
    : KParts::MainWindow( )
{
    m_part=nullptr;

    // set the shell's ui resource file
    setXMLFile(QLatin1String("cantor_shell.rc"));

    // then, setup our actions
    setupActions();

    createGUI(nullptr);

    m_tabWidget=new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(activateWorksheet(int)));
    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowTabbedDocks|QMainWindow::VerticalTabs);
}

CantorShell::~CantorShell()
{

}

void CantorShell::load(const QUrl &url)
{
    if (!m_part||!m_part->url().isEmpty() || m_part->isModified() )
    {
        addWorksheet(QLatin1String("null"));
        m_tabWidget->setCurrentIndex(m_parts.size()-1);
    }
    m_part->openUrl( url );
}

bool CantorShell::hasAvailableBackend()
{
    bool hasBackend=false;
    foreach(Cantor::Backend* b, Cantor::Backend::availableBackends())
    {
        if(b->isEnabled())
            hasBackend=true;
    }

    return hasBackend;
}

void CantorShell::setupActions()
{
    QAction* openNew = KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    openNew->setPriority(QAction::LowPriority);
    QAction* open = KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    open->setPriority(QAction::LowPriority);

    KStandardAction::close (this,  SLOT(closeTab()),  actionCollection());

    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());

    createStandardStatusBarAction();
    //setStandardToolBarMenuEnabled(true);

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());

    QAction * downloadExamples = new QAction(i18n("Download Example Worksheets"), actionCollection());
    downloadExamples->setIcon(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")));
    actionCollection()->addAction(QLatin1String("file_example_download"),  downloadExamples);
    connect(downloadExamples, SIGNAL(triggered()), this,  SLOT(downloadExamples()));

    QAction * openExample =new QAction(i18n("&Open Example"), actionCollection());
    openExample->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    actionCollection()->addAction(QLatin1String("file_example_open"), openExample);
    connect(openExample, SIGNAL(triggered()), this, SLOT(openExample()));
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

void CantorShell::fileNew()
{
    if (sender()->inherits("QAction"))
    {
        QAction * a = qobject_cast<QAction*>(sender());
        QString backendName = a->data().toString();
        if (!backendName.isEmpty())
        {
            addWorksheet(backendName);
            return;
        }
    }
    addWorksheet();
}

void CantorShell::optionsConfigureKeys()
{
    KShortcutsDialog dlg( KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, this );
    dlg.addCollection( actionCollection(), i18n("Cantor") );
    dlg.addCollection( m_part->actionCollection(), i18n("Cantor") );
    dlg.configure( true );
}

void CantorShell::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Open file"), QUrl(), i18n("*.cws|Cantor Worksheet"));

    if (url.isEmpty() == false)
    {
        // About this function, the style guide (
        // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
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
        load( url );
    }
}

void CantorShell::addWorksheet()
{
    if(hasAvailableBackend()) //There is no point in asking for the backend, if no one is available
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
        QTextBrowser *browser=new QTextBrowser(this);
        QString backendList=QLatin1String("<ul>");
        int backendListSize = 0;
        foreach(Cantor::Backend* b, Cantor::Backend::availableBackends())
        {
            if(!b->requirementsFullfilled()) //It's disabled because of misssing dependencies, not because of some other reason(like eg. nullbackend)
            {
                backendList+=QString::fromLatin1("<li>%1: <a href=\"%2\">%2</a></li>").arg(b->name(), b->url());
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
    KPluginFactory* factory = KPluginLoader(QLatin1String("libcantorpart")).factory();
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        KParts::ReadWritePart* part = factory->create<KParts::ReadWritePart>(m_tabWidget, QVariantList()<<backendName);

        if (part)
        {
            connect(part, SIGNAL(setWindowCaption(const QString&)), this, SLOT(setTabCaption(const QString&)));
            m_parts.append(part);

            //determine the icon of the backend to be added
            Cantor::Backend* backend = nullptr;
            for (Cantor::Backend* b : Cantor::Backend::availableBackends())
            {
                if (b->name() == backendName)
                {
                    backend = b;
                    break;
                }
            }

            int tab = -1;
            if (backend)
                tab = m_tabWidget->addTab(part->widget(), QIcon::fromTheme(backend->icon()), i18n("Session %1", sessionCount++));
            else //should never happend but just in case we still have some bugs somewhere...
                tab = m_tabWidget->addTab(part->widget(), i18n("Session %1", sessionCount++));

            m_tabWidget->setCurrentIndex(tab);
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
        KMessageBox::error(this, i18n("Could not find the Cantor Part."));
        qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

}

void CantorShell::activateWorksheet(int index)
{
    QObject* pluginHandler=m_part->findChild<QObject*>(QLatin1String("PanelPluginHandler"));
    disconnect(pluginHandler,SIGNAL(pluginsChanged()), this, SLOT(updatePanel()));

    m_part=findPart(m_tabWidget->widget(index));
    if(m_part)
    {
        createGUI(m_part);

        QObject* pluginHandler=m_part->findChild<QObject*>(QLatin1String("PanelPluginHandler"));
        connect(pluginHandler, SIGNAL(pluginsChanged()), this, SLOT(updatePanel()));
        updatePanel();
    }
    else
        qDebug()<<"selected part doesn't exist";

    m_tabWidget->setCurrentIndex(index);
}

void CantorShell::setTabCaption(const QString& caption)
{
    if (caption.isEmpty()) return;

    KParts::ReadWritePart* part=dynamic_cast<KParts::ReadWritePart*>(sender());
    m_tabWidget->setTabText(m_tabWidget->indexOf(part->widget()), caption);
}

void CantorShell::closeTab(int index)
{
    if (!reallyClose(false))
    {
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

    if(widget->objectName()==QLatin1String("ErrorMessage"))
    {
        widget->deleteLater();
    }else
    {
        KParts::ReadWritePart* part= findPart(widget);
        if(part)
        {
            m_parts.removeAll(part);
            delete part;
        }
    }
}

bool CantorShell::reallyClose(bool checkAllParts) {
    if(checkAllParts && m_parts.count() > 1) {
        bool modified = false;
         foreach( KParts::ReadWritePart* const part, m_parts)
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
    if (m_part && m_part->isModified() ) {
        int want_save = KMessageBox::warningYesNoCancel( this,
            i18n("The current project has been modified. Do you want to save it?"),
            i18n("Save Project"));
        switch (want_save) {
            case KMessageBox::Yes:
                m_part->save();
                if(m_part->waitSaveComplete()) {
                    return true;
                } else {
                    m_part->setModified(true);
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
    if(!reallyClose()) {
        event->ignore();
    } else {
        KParts::MainWindow::closeEvent(event);
    }
}

void CantorShell::showSettings()
{
    KConfigDialog *dialog = new KConfigDialog(this,  QLatin1String("settings"), Settings::self());
    QWidget *generalSettings = new QWidget;
    Ui::SettingsBase base;
    base.setupUi(generalSettings);
    base.kcfg_DefaultBackend->addItems(Cantor::Backend::listAvailableBackends());

    dialog->addPage(generalSettings, i18n("General"), QLatin1String("preferences-other"));
    foreach(Cantor::Backend* backend, Cantor::Backend::availableBackends())
    {
        if (backend->config()) //It has something to configure, so add it to the dialog
            dialog->addPage(backend->settingsWidget(dialog), backend->config(), backend->name(),  backend->icon());
    }

    dialog->show();
}

void CantorShell::downloadExamples()
{
    KNS3::DownloadDialog dialog;
    dialog.exec();
    foreach (const KNS3::Entry& e,  dialog.changedEntries())
    {
        qDebug() << "Changed Entry: " << e.name();
    }
}

void CantorShell::openExample()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/examples");
    if (dir.isEmpty()) return;
    QDir().mkpath(dir);

    QStringList files=QDir(dir).entryList(QDir::Files);
    QPointer<QDialog> dlg=new QDialog(this);
    QListWidget* list=new QListWidget(dlg);
    foreach(const QString& file, files)
    {
        QString name=file;
        name.remove(QRegExp(QLatin1String("-.*\\.hotstuff-access$")));
        list->addItem(name);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(list);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    connect(buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()) );
    connect(buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()) );

    if (dlg->exec()==QDialog::Accepted&&list->currentRow()>=0)
    {
        const QString& selectedFile=files[list->currentRow()];
        QUrl url = QUrl::fromLocalFile(QDir(dir).absoluteFilePath(selectedFile));

        qDebug()<<"loading file "<<url;
        load(url);
    }

    delete dlg;
}

KParts::ReadWritePart* CantorShell::findPart(QWidget* widget)
{
    foreach( KParts::ReadWritePart* const part, m_parts)
    {
        if(part->widget()==widget)
            return part;
    }
    return nullptr;
}

void CantorShell::updatePanel()
{
    unplugActionList(QLatin1String("view_show_panel_list"));

    //remove all of the previous panels (but do not delete the widgets)
    foreach(QDockWidget* dock, m_panels)
    {
        QWidget* widget=dock->widget();
        if(widget!=nullptr)
        {
            widget->setParent(this);
            widget->hide();
        }
        dock->deleteLater();
    }
    m_panels.clear();

    QList<QAction*> panelActions;

    Cantor::PanelPluginHandler* handler=m_part->findChild<Cantor::PanelPluginHandler*>(QLatin1String("PanelPluginHandler"));
    if(!handler)
    {
        qDebug()<<"no PanelPluginHandle found for this part";
        return;
    }

    QDockWidget* last=nullptr;

    QList<Cantor::PanelPlugin*> plugins=handler->plugins();
    foreach(Cantor::PanelPlugin* plugin, plugins)
    {
        if(plugin==nullptr)
        {
            qDebug()<<"somethings wrong";
            continue;
        }

        qDebug()<<"adding panel for "<<plugin->name();
        plugin->setParentWidget(this);

        QDockWidget* docker=new QDockWidget(plugin->name(), this);
        docker->setObjectName(plugin->name());
        docker->setWidget(plugin->widget());
        addDockWidget ( Qt::RightDockWidgetArea,  docker );

        if(last!=nullptr)
            tabifyDockWidget(last, docker);
        last=docker;

        connect(plugin, SIGNAL(visibilityRequested()), docker, SLOT(raise()));

        m_panels.append(docker);

        //Create the action to show/hide this panel
        panelActions<<docker->toggleViewAction();

    }

    plugActionList(QLatin1String("view_show_panel_list"), panelActions);

    unplugActionList(QLatin1String("new_worksheet_with_backend_list"));
    QList<QAction*> newBackendActions;
    foreach (Cantor::Backend* backend, Cantor::Backend::availableBackends())
    {
        if (!backend->isEnabled())
            continue;
        QAction * action = new QAction(QIcon::fromTheme(backend->icon()), backend->name(), nullptr);
        action->setData(backend->name());
        connect(action, SIGNAL(triggered()), this, SLOT(fileNew()));
        newBackendActions << action;
    }
    plugActionList(QLatin1String("new_worksheet_with_backend_list"), newBackendActions);
}

Cantor::WorksheetAccessInterface* CantorShell::currentWorksheetAccessInterface()
{
    Cantor::WorksheetAccessInterface* wa=m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);

    if (!wa)
        qDebug()<<"failed to access worksheet access interface for current part";

    return wa;
}
