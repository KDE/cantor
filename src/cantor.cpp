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
#include "cantor.moc"

#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kshortcutsdialog.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <kdebug.h>
#include <kconfigdialog.h>
#include <ktextedit.h>
#include <ktextbrowser.h>
#include <kxmlguifactory.h>
#include <kstandarddirs.h>
#include <ktoggleaction.h>

#include <knewstuff3/downloaddialog.h>

#include <QDockWidget>
#include <QApplication>

#include "lib/backend.h"

#include "settings.h"
#include "ui_settings.h"
#include "backendchoosedialog.h"

CantorShell::CantorShell()
    : KParts::MainWindow( )
{
    m_part=0;

    // set the shell's ui resource file
    setXMLFile("cantor_shell.rc");

    // then, setup our actions
    setupActions();

    m_helpDocker=new QDockWidget(i18n("Help"), this);
    m_helpDocker->setObjectName("help");
    m_helpView=new KTextEdit(m_helpDocker);
    m_helpView->setText(i18n("<h1>Cantor</h1>The KDE way to do Mathematics"));
    m_helpView->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_helpDocker->setWidget(m_helpView);
    addDockWidget ( Qt::RightDockWidgetArea,  m_helpDocker );

    KToggleAction* showHelpPanelAction=new KToggleAction(i18n("Show Help Panel"), actionCollection());
    showHelpPanelAction->setIcon(KIcon("help-hint"));
    connect(showHelpPanelAction, SIGNAL(toggled(bool)), this, SLOT(showHelpDocker(bool)));
    connect(m_helpDocker, SIGNAL(visibilityChanged(bool)), showHelpPanelAction, SLOT(setChecked(bool)));
    actionCollection()->addAction("show_help_panel", showHelpPanelAction);

    createGUI(0);

    m_tabWidget=new KTabWidget(this);
    m_tabWidget->setCloseButtonEnabled(true);
    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(activateWorksheet(int)));
    connect(m_tabWidget, SIGNAL(closeRequest (QWidget *)), this, SLOT(closeTab(QWidget*)));

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();
}

CantorShell::~CantorShell()
{

}

void CantorShell::load(const KUrl& url)
{
    if (!m_part||!m_part->url().isEmpty() || m_part->isModified() )
    {
        addWorksheet("null");
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
    KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());

    KStandardAction::close (this,  SLOT(closeTab()),  actionCollection());

    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());

    createStandardStatusBarAction();
    //setStandardToolBarMenuEnabled(true);

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());

    KAction* downloadExamples = new KAction(i18n("Download Example Worksheets"), actionCollection());
    downloadExamples->setIcon(KIcon("get-hot-new-stuff"));
    actionCollection()->addAction("file_example_download",  downloadExamples);
    connect(downloadExamples, SIGNAL(triggered()), this,  SLOT(downloadExamples()));

    KAction* openExample =new KAction(i18n("&Open Example"), actionCollection());
    openExample->setIcon(KIcon("document-open"));
    actionCollection()->addAction("file_example_open", openExample);
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
    KUrl url = KFileDialog::getOpenUrl( KUrl(), i18n("*.cws|Cantor Worksheet"), this );

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
        QPointer<BackendChooseDialog> dlg=new BackendChooseDialog(this);
        if(dlg->exec())
        {
            addWorksheet(dlg->backendName());
            activateWorksheet(m_parts.size()-1);
        }

        delete dlg;
    }else
    {
        KTextBrowser *browser=new KTextBrowser(this);
        QString backendList="<ul>";
        int backendListSize = 0;
        foreach(Cantor::Backend* b, Cantor::Backend::availableBackends())
        {
            if(!b->requirementsFullfilled()) //It's disabled because of misssing dependencies, not because of some other reason(like eg. nullbackend)
            {
                backendList+=QString("<li>%1: <a href=\"%2\">%2</a></li>").arg(b->name(), b->url());
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

        browser->setObjectName("ErrorMessage");
        m_tabWidget->addTab(browser, i18n("Error"));
    }
}

void CantorShell::addWorksheet(const QString& backendName)
{
    static int sessionCount=1;

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libcantorpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        KParts::ReadWritePart* part = dynamic_cast<KParts::ReadWritePart *>(factory->create(m_tabWidget, "CantorPart", QStringList()<<backendName ));

        if (part)
        {
            connect(part, SIGNAL(showHelp(const QString&)), m_helpView, SLOT(setHtml(const QString&)));
            connect(part, SIGNAL(setCaption(const QString&)), this, SLOT(setTabCaption(const QString&)));
            m_parts.append(part);
            m_tabWidget->addTab(part->widget(), i18n("Session %1", sessionCount++));
        }
        else
        {
            kDebug()<<"error creating part ";
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
    m_part=findPart(m_tabWidget->widget(index));
    if(m_part)
        createGUI(m_part);
    else
        kDebug()<<"selected part doesn't exist";

    m_tabWidget->setCurrentIndex(index);
}

void CantorShell::setTabCaption(const QString& caption)
{
    if (caption.isEmpty()) return;

    KParts::ReadWritePart* part=dynamic_cast<KParts::ReadWritePart*>(sender());
    m_tabWidget->setTabText(m_tabWidget->indexOf(part->widget()), caption);
}

void CantorShell::closeTab(QWidget* widget)
{
    if(widget==0)
    {
        if(m_part!=0)
            widget=m_part->widget();
        else
            return;
    }

    int index=m_tabWidget->indexOf(widget);

    m_tabWidget->removeTab(index);

    if(widget->objectName()=="ErrorMessage")
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

void CantorShell::showSettings()
{
    KConfigDialog *dialog = new KConfigDialog(this,  "settings", Settings::self());
    QWidget *generalSettings = new QWidget;
    Ui::SettingsBase base;
    base.setupUi(generalSettings);
    base.kcfg_DefaultBackend->addItems(Cantor::Backend::listAvailableBackends());

    dialog->addPage(generalSettings, i18n("General"), "preferences-other");
    foreach(Cantor::Backend* backend, Cantor::Backend::availableBackends())
    {
        if (backend->config()) //It has something to configure, so add it to the dialog
            dialog->addPage(backend->settingsWidget(dialog), backend->config(), backend->name(),  backend->icon());
    }

    dialog->show();
}

void CantorShell::showHelpDocker(bool show)
{
    if(show!=m_helpDocker->isVisible())
    {
        m_helpDocker->setVisible(show);
    }
}

void CantorShell::downloadExamples()
{
    KNS3::DownloadDialog dialog;
    dialog.exec();
    foreach (const KNS3::Entry& e,  dialog.changedEntries())
    {
        kDebug() << "Changed Entry: " << e.name();
    }
}

void CantorShell::openExample()
{
    QString dir = KStandardDirs::locateLocal("appdata",  "examples");
    if (dir.isEmpty()) return;
    KStandardDirs::makeDir(dir);

    QStringList files=QDir(dir).entryList(QDir::Files);
    QPointer<KDialog> dlg=new KDialog(this);
    QListWidget* list=new QListWidget(dlg);
    foreach(const QString& file, files)
    {
        QString name=file;
        name.remove(QRegExp("-.*\\.hotstuff-access$"));
        list->addItem(name);
    }

    dlg->setMainWidget(list);

    if (dlg->exec()==QDialog::Accepted&&list->currentRow()>=0)
    {
        const QString& selectedFile=files[list->currentRow()];
        KUrl url;
        url.setDirectory(dir);
        url.setFileName(selectedFile);

        kDebug()<<"loading file "<<url;
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
    return 0;
}
