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
#include "mathematik.h"
#include "mathematik.moc"

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

#include <QDockWidget>
#include <QApplication>
#include <QTimer>

#include "helpextension.h"
#include "lib/backend.h"

#include "settings.h"
#include "ui_settings.h"
#include "backendchoosedialog.h"

MathematiKShell::MathematiKShell()
    : KParts::MainWindow( )
{
    // set the shell's ui resource file
    setXMLFile("mathematik_shell.rc");

    // then, setup our actions
    setupActions();

    QDockWidget* dock=new QDockWidget(i18n("Help"), this);
    m_helpView=new KTextEdit(dock);
    m_helpView->setText(i18n("<h1>MathematiK</h1>The KDE way to do Mathematics"));
    m_helpView->setTextInteractionFlags(Qt::TextEditable);
    dock->setWidget(m_helpView);
    addDockWidget ( Qt::RightDockWidgetArea,  dock );

    m_tabWidget=new KTabWidget(this);
    m_tabWidget->setCloseButtonEnabled(true);
    setCentralWidget(m_tabWidget);
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(activateWorksheet(int)));
    connect(m_tabWidget, SIGNAL(closeRequest (QWidget *)), this, SLOT(closeTab(QWidget*)));

    createGUI(0);
    QTimer::singleShot(0, this, SLOT(addWorksheet()));

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();
}

MathematiKShell::~MathematiKShell()
{
}

void MathematiKShell::load(const KUrl& url)
{
    if (!m_part->url().isEmpty() || m_part->isModified() )
    {
        addWorksheet("nullbackend");
        m_tabWidget->setCurrentIndex(m_parts.size()-1);
    }
    m_part->openUrl( url );
}

void MathematiKShell::setupActions()
{
    KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());

    KStandardAction::close (this,  SLOT(closeTab()),  actionCollection());

    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());

    createStandardStatusBarAction();
    //setStandardToolBarMenuEnabled(true);

    //KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    //KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
}

void MathematiKShell::saveProperties(KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void MathematiKShell::readProperties(const KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void MathematiKShell::fileNew()
{
    addWorksheet();
}

void MathematiKShell::optionsConfigureKeys()
{
  /*KShortcutsDialog dlg( KKeyChooser::AllActions, KKeyChooser::LetterShortcutsDisallowed, this );
  dlg.insert( actionCollection(), "mathematik_shell.rc" );
  dlg.insert( m_part->actionCollection(), "mathematik_part.rc" );
  (void) dlg.configure( true );*/
}

void MathematiKShell::optionsConfigureToolbars()
{
    //saveMainWindowSettings(KGlobal::config(), autoSaveGroup());

    // use the standard toolbar editor
    /*KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();*/
}

void MathematiKShell::applyNewToolbarConfig()
{
    //applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void MathematiKShell::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    KUrl url =
        KFileDialog::getOpenUrl( KUrl(), QString("*.mws"), this );

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
            MathematiKShell* newWin = new MathematiKShell;
            newWin->load( url );
            newWin->show();
            }*/
        load( url );
    }
}

void MathematiKShell::addWorksheet()
{
    BackendChooseDialog dlg(this);
    if(dlg.exec())
    {
        addWorksheet(dlg.backendName());
        activateWorksheet(m_parts.size()-1);
    }
}

void MathematiKShell::addWorksheet(const QString& backendName)
{
    static int sessionCount=1;

    statusBar()->showMessage(i18n("Initializing session"));
    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libmathematikpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        KParts::ReadWritePart* part = dynamic_cast<KParts::ReadWritePart *>(factory->create(m_tabWidget, "MathematiKPart", QStringList()<<backendName ));

        if (part)
        {
            QObject* ext=(part->findChild<QObject*>("helpExtension"));
            connect(ext, SIGNAL(showHelp(const QString&)), m_helpView, SLOT(setHtml(const QString&)));
            connect(part, SIGNAL(setWindowCaption(const QString&)), this, SLOT(setTabCaption(const QString&)));
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
        KMessageBox::error(this, i18n("Could not find the Mathematik Part."));
        qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

}

void MathematiKShell::activateWorksheet(int index)
{
    m_part=m_parts.value(index);
    if(m_part)
        createGUI(m_part);
    else
        kDebug()<<"selected part doesnt exist";

    m_tabWidget->setCurrentIndex(index);
}

void MathematiKShell::setTabCaption(const QString& caption)
{
    if (caption.isEmpty()) return;

    KParts::ReadWritePart* part=dynamic_cast<KParts::ReadWritePart*>(sender());
    m_tabWidget->setTabText(m_tabWidget->indexOf(part->widget()), caption);
}

void MathematiKShell::closeTab(QWidget* widget)
{
    if(widget==0) widget=m_part->widget();
    int index=m_tabWidget->indexOf(widget);

    KParts::ReadWritePart* part=m_parts.takeAt(index);
    m_tabWidget->removeTab(index);

    part->deleteLater();
}

void MathematiKShell::showSettings()
{
    KConfigDialog *dialog = new KConfigDialog(this,  "settings", Settings::self());
    QWidget *generalSettings = new QWidget;
    Ui::SettingsBase base;
    base.setupUi(generalSettings);
    base.kcfg_DefaultBackend->addItems(MathematiK::Backend::listAvailableBackends());

    dialog->addPage(generalSettings, i18n("General"), "preferences-other");
    foreach(const QString& name, MathematiK::Backend::listAvailableBackends())
    {
        MathematiK::Backend* backend=MathematiK::Backend::createBackend(name);
        if (backend->config()) //It has something to configure, so add it to the dialog
            dialog->addPage(backend->settingsWidget(dialog), backend->config(), backend->name(),  backend->icon());
    }

    dialog->show();
}
