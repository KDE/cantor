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

#include "cantor_part.h"

#include "cantor_part.moc"

#include <config-cantor.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kparts/genericfactory.h>
#include <kparts/event.h>
#include <kstandardaction.h>
#include <kzip.h>
#include <ktoggleaction.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <krun.h>
#include <kprogressdialog.h>
#include <kmessagebox.h>
#include <knewstuff3/uploaddialog.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QTextEdit>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

#include "worksheet.h"
#include "scripteditorwidget.h"
#include "lib/backend.h"
#include "lib/extension.h"
#include "lib/assistant.h"
#include "lib/panelpluginhandler.h"
#include "lib/panelplugin.h"

#include "settings.h"

K_PLUGIN_FACTORY(CantorPartFactory, registerPlugin<CantorPart>();)
K_EXPORT_PLUGIN(CantorPartFactory("cantor"))

CantorPart::CantorPart( QWidget *parentWidget, QObject *parent, const QVariantList & args ): KParts::ReadWritePart(parent)
{
    // we need an instance
    setComponentData( CantorPartFactory::componentData() );

    m_showBackendHelp=0;
    m_initProgressDlg=0;
    m_statusBarBlocked=false;

    m_panelHandler=new Cantor::PanelPluginHandler(this);
    connect(m_panelHandler, SIGNAL(pluginsChanged()), this, SLOT(pluginsChanged()));

    kDebug()<<"Created a CantorPart";
    QString backendName;
    if(args.isEmpty())
        backendName="null";
    else
        backendName=args.first().toString();

    Cantor::Backend* b=Cantor::Backend::createBackend(backendName);
    if(!b)
    {
        KMessageBox::error(parentWidget, i18n("Backend %1 is not installed", backendName), i18n("Error - Cantor"));
        setWidget(new QWidget(parentWidget));
        //fake being modified so the shell won't try to reuse this part
        ReadWritePart::setModified(true);
        return;
    }

    kDebug()<<"Backend "<<b->name()<<" offers extensions: "<<b->extensions();

    m_worksheet=new Worksheet(b, parentWidget);
    m_worksheet->setEnabled(false); //disable input until the session has successfully logged in and emits the ready signal
    connect(m_worksheet, SIGNAL(modified()), this, SLOT(setModified()));
    connect(m_worksheet, SIGNAL(showHelp(const QString&)), this, SIGNAL(showHelp(const QString&)));
    connect(m_worksheet, SIGNAL(sessionChanged()), this, SLOT(worksheetSessionChanged()));

    // notify the part that this is our internal widget
    setWidget(m_worksheet);

    // create our actions
    m_worksheet->createActions( actionCollection() );

    KStandardAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    m_save = KStandardAction::save(this, SLOT(save()), actionCollection());

    KAction* latexExport=new KAction(i18n("Export to LaTex"), actionCollection());
    actionCollection()->addAction("file_export_latex", latexExport);
    latexExport->setIcon(KIcon("document-export"));
    connect(latexExport, SIGNAL(triggered()), this, SLOT(exportToLatex()));

    KStandardAction::print(this, SLOT(print()), actionCollection());

    KStandardAction::zoomIn(m_worksheet, SLOT(zoomIn()), actionCollection());
    KStandardAction::zoomOut(m_worksheet, SLOT(zoomOut()), actionCollection());

    m_evaluate=new KAction(i18n("Evaluate Worksheet"), actionCollection());
    actionCollection()->addAction("evaluate_worksheet", m_evaluate);
    m_evaluate->setIcon(KIcon("system-run"));
    connect(m_evaluate, SIGNAL(triggered()), this, SLOT(evaluateOrInterrupt()));

    m_typeset=new KToggleAction(i18n("Typeset using LaTeX"), actionCollection());
    m_typeset->setChecked(Settings::self()->typesetDefault());
    actionCollection()->addAction("enable_typesetting", m_typeset);
    connect(m_typeset, SIGNAL(toggled(bool)), this, SLOT(enableTypesetting(bool)));

    m_highlight=new KToggleAction(i18n("Syntax Highlighting"), actionCollection());
    m_highlight->setChecked(Settings::self()->highlightDefault());
    actionCollection()->addAction("enable_highlighting", m_highlight);
    connect(m_highlight, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableHighlighting(bool)));

    m_completion=new KToggleAction(i18n("Completion"), actionCollection());
    m_completion->setChecked(Settings::self()->completionDefault());
    actionCollection()->addAction("enable_completion", m_completion);
    connect(m_completion, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableCompletion(bool)));

    m_exprNumbering=new KToggleAction(i18n("Line Numbers"), actionCollection());
    m_exprNumbering->setChecked(Settings::self()->expressionNumberingDefault());
    actionCollection()->addAction("enable_expression_numbers", m_exprNumbering);
    connect(m_exprNumbering, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableExpressionNumbering(bool)));

    KAction* restart=new KAction(i18n("Restart Backend"), actionCollection());
    actionCollection()->addAction("restart_backend", restart);
    restart->setIcon(KIcon("system-reboot"));
    connect(restart, SIGNAL(triggered()), this, SLOT(restartBackend()));

    KAction* evaluateCurrent=new KAction(i18n("Evaluate Entry"), actionCollection());
    evaluateCurrent->setShortcut(Qt::SHIFT + Qt::Key_Return);
    actionCollection()->addAction("evaluate_current",  evaluateCurrent);
    connect(evaluateCurrent, SIGNAL(triggered()), m_worksheet, SLOT(evaluateCurrentEntry()));

    KAction* insertCommandEntry=new KAction(i18n("Insert Command Entry"), actionCollection());
    insertCommandEntry->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_command_entry",  insertCommandEntry);
    connect(insertCommandEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertCommandEntry()));

    KAction* insertTextEntry=new KAction(i18n("Insert Text Entry"), actionCollection());
    //insertEntry->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_text_entry",  insertTextEntry);
    connect(insertTextEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertTextEntry()));

    KAction* insertCommandEntryBefore=new KAction(i18n("Insert Command Entry Before"), actionCollection());
    //insertCommandEntryBefore->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_command_entry_before",  insertCommandEntryBefore);
    connect(insertCommandEntryBefore, SIGNAL(triggered()), m_worksheet, SLOT(insertCommandEntryBefore()));

    KAction* insertTextEntryBefore=new KAction(i18n("Insert Text Entry Before"), actionCollection());
    //insertTextEntryBefore->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_text_entry_before",  insertTextEntryBefore);
    connect(insertTextEntryBefore, SIGNAL(triggered()), m_worksheet, SLOT(insertTextEntryBefore()));

    KAction* removeCurrent=new KAction(i18n("Remove current Entry"), actionCollection());
    removeCurrent->setShortcut(Qt::ShiftModifier + Qt::Key_Delete);
    actionCollection()->addAction("remove_current", removeCurrent);
    connect(removeCurrent, SIGNAL(triggered()), m_worksheet, SLOT(removeCurrentEntry()));

    m_showBackendHelp=new KAction(i18n("Show %1 Help", b->name()) , actionCollection());
    m_showBackendHelp->setIcon(KIcon("help-contents"));
    actionCollection()->addAction("backend_help", m_showBackendHelp);
    connect(m_showBackendHelp, SIGNAL(triggered()), this, SLOT(showBackendHelp()));

    KAction* publishWorksheet=new KAction(i18n("Publish Worksheet"), actionCollection());
    publishWorksheet->setIcon(KIcon("get-hot-new-stuff"));
    actionCollection()->addAction("file_publish_worksheet", publishWorksheet);
    connect(publishWorksheet, SIGNAL(triggered()), this, SLOT(publishWorksheet()));

    KToggleAction* showEditor=new KToggleAction(i18n("Show Script Editor"), actionCollection());
    showEditor->setChecked(false);
    actionCollection()->addAction("show_editor", showEditor);
    connect(showEditor, SIGNAL(toggled(bool)), this, SLOT(showScriptEditor(bool)));
    showEditor->setEnabled(b->extensions().contains("ScriptExtension"));

    KAction* showCompletion=new KAction(i18n("Show Completion"), actionCollection());
    actionCollection()->addAction("show_completion", showCompletion);
    showCompletion->setShortcut(Qt::Key_Tab); //Qt::CTRL + Qt::Key_Space);
    connect(showCompletion, SIGNAL(triggered()), m_worksheet, SLOT(showCompletion()));

    // set our XML-UI resource file
    setXMLFile("cantor_part.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

    worksheetSessionChanged();
}

CantorPart::~CantorPart()
{
    if (m_scriptEditor)
    {
        disconnect(m_scriptEditor, SIGNAL(destroyed()), this, SLOT(scriptEditorClosed()));
        delete m_scriptEditor;
    }
}

void CantorPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    m_worksheet->setReadOnly(!rw);

    ReadWritePart::setReadWrite(rw);
}

void CantorPart::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    if (!m_save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        m_save->setEnabled(true);
    else
        m_save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

KAboutData *CantorPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the
    // Makefile)
    KAboutData *aboutData = new KAboutData("cantorpart",  "cantor",  ki18n("CantorPart"), "0.2");
    aboutData->addAuthor(ki18n("Alexander Rieder"), KLocalizedString(), "alexanderrieder@gmail.com");
    return aboutData;
}

bool CantorPart::openFile()
{
    //don't crash if for some reason the worksheet is invalid
    if(m_worksheet==0)
    {
        kWarning()<<"trying to open in an invalid cantor part";
        return false;
    }

    m_worksheet->load(localFilePath());

    // just for fun, set the status bar
    //setStatusMessage( m_url.prettyUrl() );

    updateCaption();

    return true;
}

bool CantorPart::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    kDebug()<<"saving to: "<<url();
    if (url().isEmpty())
        fileSaveAs();
    else
    {
        if(url().fileName().endsWith(QLatin1String(".cws")) || url().fileName().endsWith(QLatin1String(".mws")))
            m_worksheet->save( localFilePath() );
        else
            m_worksheet->savePlain( localFilePath());

    }
    setModified(false);

    return true;
}

void CantorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString filter=i18n("*.cws|Cantor Worksheet");

    //if the backend supports scripts, also append their scriptFile endings to the filter
    Cantor::Backend * const backend=m_worksheet->session()->backend();
    if (backend->extensions().contains("ScriptExtension"))
    {
        Cantor::ScriptExtension* e=dynamic_cast<Cantor::ScriptExtension*>(backend->extension("ScriptExtension"));
        filter+='\n'+e->scriptFileFilter();
    }

    QString file_name = KFileDialog::getSaveFileName(KUrl(), filter, widget());
    if (file_name.isEmpty() == false)
        saveAs(file_name);

    updateCaption();
}

void CantorPart::exportToLatex()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString filter=i18n("*.tex|LaTex Document");

    QString file_name = KFileDialog::getSaveFileName(KUrl(), filter, widget());

    if (file_name.isEmpty() == false)
    {
        int exportImages=KMessageBox::questionYesNo(widget(), i18n("Do you also want to export the images?"), i18n("Question - Cantor"));
        m_worksheet->saveLatex( file_name , exportImages==KMessageBox::Yes);
    }
}

void CantorPart::guiActivateEvent( KParts::GUIActivateEvent * event )
{
    KParts::ReadWritePart::guiActivateEvent(event);
    if(event->activated())
    {
        if(m_scriptEditor)
            m_scriptEditor->show();
    }else
    {
        if(m_scriptEditor)
            m_scriptEditor->hide();
    }
}

void CantorPart::evaluateOrInterrupt()
{
    kDebug()<<"evalorinterrupt";
    if(m_worksheet->isRunning())
        m_worksheet->interrupt();
    else
        m_worksheet->evaluate();
}
void CantorPart::restartBackend()
{
    m_worksheet->session()->logout();
    m_worksheet->session()->login();
}

void CantorPart::worksheetStatusChanged(Cantor::Session::Status status)
{
    kDebug()<<"wsStatusChange"<<status;
    if(status==Cantor::Session::Running)
    {
        m_evaluate->setText(i18n("Interrupt"));
        m_evaluate->setIcon(KIcon("dialog-close"));

        setStatusMessage(i18n("Calculating..."));
    }else
    {
        m_evaluate->setText(i18n("Evaluate Worksheet"));
        m_evaluate->setIcon(KIcon("system-run"));

        setStatusMessage(i18n("Ready"));
    }
}

void CantorPart::showSessionError(const QString& message)
{
    kDebug()<<"Error: "<<message;
    initialized();
    showImportantStatusMessage(i18n("Session Error: %1", message));
}

void CantorPart::worksheetSessionChanged()
{
    connect(m_worksheet->session(), SIGNAL(statusChanged(Cantor::Session::Status)), this, SLOT(worksheetStatusChanged(Cantor::Session::Status)));
    connect(m_worksheet->session(), SIGNAL(ready()),this, SLOT(initialized()));
    connect(m_worksheet->session(), SIGNAL(error(const QString&)), this, SLOT(showSessionError(const QString&)));

    loadAssistants();
    m_panelHandler->setSession(m_worksheet->session());
    adjustGuiToSession();

    if(!m_initProgressDlg)
    {
        m_initProgressDlg=new KProgressDialog(widget(), i18n("Cantor"), i18n("Initializing Session"));
        m_initProgressDlg->setMinimumDuration(500);
        m_initProgressDlg->progressBar()->setRange(0, 0);
    }
}

void CantorPart::initialized()
{
    m_worksheet->appendCommandEntry();
    m_worksheet->setEnabled(true);
    m_worksheet->setFocus();
    setStatusMessage(i18n("Initialization complete"));

    if(m_initProgressDlg)
    {
        m_initProgressDlg->deleteLater();
        m_initProgressDlg=0;
    }
    updateCaption();
}

void CantorPart::enableTypesetting(bool enable)
{
    m_worksheet->session()->setTypesettingEnabled(enable);
}

void CantorPart::showBackendHelp()
{
    kDebug()<<"showing backends help";
    Cantor::Backend* backend=m_worksheet->session()->backend();
    KUrl url=backend->helpUrl();
    kDebug()<<"launching url "<<url;
    new KRun(url, widget());
}

Worksheet* CantorPart::worksheet()
{
    return m_worksheet;
}

void CantorPart::updateCaption()
{
    QString filename=url().fileName();
    //strip away the extension
    filename=filename.left(filename.lastIndexOf('.'));

    if (filename.isEmpty())
        filename=i18n("Unnamed");

    emit setCaption(i18n("%1: %2", m_worksheet->session()->backend()->name(), filename));
}

void CantorPart::pluginsChanged()
{
    foreach(Cantor::PanelPlugin* plugin, m_panelHandler->plugins())
    {
        connect(plugin, SIGNAL(requestRunCommand(QString)), this, SLOT(runCommand(QString)));
    }
}

void CantorPart::loadAssistants()
{
    kDebug()<<"loading assistants...";

    KService::List services;
    KServiceTypeTrader* trader = KServiceTypeTrader::self();

    services = trader->query("Cantor/Assistant");

    foreach (const KService::Ptr &service,   services)
    {
        QString error;

        kDebug()<<"found service"<<service->name();
        Cantor::Assistant* assistant=service->createInstance<Cantor::Assistant>(this,  QVariantList(),   &error);
        if (assistant==0)
        {
            kDebug()<<"error loading assistant"<<service->name()<<":  "<<error;
            continue;
        }

        kDebug()<<"created it";
        Cantor::Backend* backend=worksheet()->session()->backend();
        KPluginInfo info(service);
        assistant->setPluginInfo(info);
        assistant->setBackend(backend);

        kDebug()<<"plugin "<<service->name()<<" requires "<<assistant->requiredExtensions();
        bool supported=true;
        foreach(const QString& req, assistant->requiredExtensions())
            supported=supported && backend->extensions().contains(req);
        kDebug()<<"plugin "<<service->name()<<" is "<<(supported ? "":" not ")<<" supported by "<<backend->name();

        if(supported)
        {
            assistant->initActions();
            //createGui(assistant);
            connect(assistant, SIGNAL(requested()), this, SLOT(runAssistant()));
        }else
        {
            removeChildClient(assistant);
            assistant->deleteLater();
        }
    }


}

void CantorPart::runAssistant()
{
    Cantor::Assistant* a=qobject_cast<Cantor::Assistant*>(sender());
    QStringList cmds=a->run(widget());
    kDebug()<<cmds;
    if(!cmds.isEmpty())
        runCommand(cmds.join("\n"));
}

void CantorPart::runCommand(const QString& cmd)
{
    m_worksheet->appendCommandEntry(cmd);
}

void CantorPart::adjustGuiToSession()
{
#ifdef WITH_EPS
    m_typeset->setVisible(m_worksheet->session()->backend()->capabilities().testFlag(Cantor::Backend::LaTexOutput));
#else
    m_typeset->setVisible(false);
#endif
    m_completion->setVisible(m_worksheet->session()->backend()->capabilities().testFlag(Cantor::Backend::Completion));

    //this is 0 on the first call
    if(m_showBackendHelp)
        m_showBackendHelp->setText(i18n("Show %1 Help", m_worksheet->session()->backend()->name()));
}

void CantorPart::publishWorksheet()
{
    int ret = KMessageBox::questionYesNo(widget(),
                                         i18n("Do you want to upload current Worksheet to public web server?"),
                                         i18n("Question - Cantor"));
    if (ret != KMessageBox::Yes) return;

    if (isModified()||url().isEmpty())
    {
        ret = KMessageBox::warningContinueCancel(widget(),
                                                 i18n("The Worksheet is not saved. You should save it before uploading."),
                                                 i18n("Warning - Cantor"),  KStandardGuiItem::save(),  KStandardGuiItem::cancel());
        if (ret != KMessageBox::Continue) return;
        if (!saveFile()) return;
    }

    kDebug()<<"uploading file "<<url();

    // upload
    //HACK: use different .knsrc files for each category
    //remove this once KNS3 gains the ability to select category
    KNS3::UploadDialog dialog(QString("cantor_%1.knsrc").arg(m_worksheet->session()->backend()->id().toLower()), widget());
    dialog.setUploadFile(url());
    dialog.exec();
}

void CantorPart::print()
{
    QPrinter printer;
    QPointer<QPrintDialog> dialog = new QPrintDialog(&printer,  widget());

    if (m_worksheet->textCursor().hasSelection())
        dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);

    if (dialog->exec() == QDialog::Accepted)
        m_worksheet->print(&printer);

    delete dialog;
}

void CantorPart::showScriptEditor(bool show)
{
    if(show)
    {
        if (m_scriptEditor)
        {
            return;
        }
        Cantor::ScriptExtension* scriptE=dynamic_cast<Cantor::ScriptExtension*>(m_worksheet->session()->backend()->extension("ScriptExtension"));
        if (!scriptE)
        {
            return;
        }
        m_scriptEditor=new ScriptEditorWidget(scriptE->scriptFileFilter(), widget()->window());
        connect(m_scriptEditor, SIGNAL(runScript(const QString&)), this, SLOT(runScript(const QString&)));
        connect(m_scriptEditor, SIGNAL(destroyed()), this, SLOT(scriptEditorClosed()));
        m_scriptEditor->show();
    }else
    {
        delete m_scriptEditor;
    }
}

void CantorPart::scriptEditorClosed()
{
    QAction* showEditor = actionCollection()->action("show_editor");
    if (showEditor)
    {
        showEditor->setChecked(false);
    }
}

void CantorPart::runScript(const QString& file)
{
    Cantor::Backend* backend=m_worksheet->session()->backend();
    if(!backend->extensions().contains("ScriptExtension"))
    {
        KMessageBox::error(widget(), i18n("This backend does not support scripts."), i18n("Error - Cantor"));
        return;
    }

    Cantor::ScriptExtension* scriptE=dynamic_cast<Cantor::ScriptExtension*>(backend->extension("ScriptExtension"));
    m_worksheet->appendCommandEntry(scriptE->runExternalScript(file));
}

void CantorPart::blockStatusBar()
{
    m_statusBarBlocked=true;
}

void CantorPart::unblockStatusBar()
{
    m_statusBarBlocked=false;
    if(!m_cachedStatusMessage.isNull())
        setStatusMessage(m_cachedStatusMessage);
    m_cachedStatusMessage.clear();
}

void CantorPart::setStatusMessage(const QString& message)
{
    if(!m_statusBarBlocked)
        emit setStatusBarText(message);
    else
        m_cachedStatusMessage=message;
}

void CantorPart::showImportantStatusMessage(const QString& message)
{
    setStatusMessage(message);
    blockStatusBar();
    QTimer::singleShot(3000, this, SLOT(unblockStatusBar()));
}
