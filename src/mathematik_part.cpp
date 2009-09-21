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

#include "mathematik_part.h"

#include "mathematik_part.moc"

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
#include <knewstuff2/engine.h>

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

#include "helpextension.h"
#include "settings.h"

typedef KParts::GenericFactory<MathematiKPart> MathematiKPartFactory;
K_EXPORT_COMPONENT_FACTORY( libmathematikpart, MathematiKPartFactory )

MathematiKPart::MathematiKPart( QWidget *parentWidget, QObject *parent, const QStringList & args ): KParts::ReadWritePart(parent)
{
    // we need an instance
    setComponentData( MathematiKPartFactory::componentData() );

    m_showBackendHelp=0;
    m_initProgressDlg=0;
    m_scriptEditor=0;

    HelpExtension* h=new HelpExtension(this);

    kDebug()<<"Created a MathematiKPart";
    QString backendName;
    if(args.isEmpty())
        backendName="nullbackend";
    else
        backendName=args.first();

    MathematiK::Backend* b=MathematiK::Backend::createBackend(backendName);
    if(!b)
    {
        KMessageBox::error(parentWidget, i18n("Backend %1 is not installed", backendName), i18n("Error - MathematiK"));
        setWidget(new QWidget(parentWidget));
        return;
    }

    kDebug()<<"Backend "<<b->name()<<" offers extensions: "<<b->extensions();

    m_worksheet=new Worksheet(b, parentWidget);
    m_worksheet->setEnabled(false); //disable input until the session has successfully logged in and emits the ready signal
    connect(m_worksheet, SIGNAL(modified()), this, SLOT(setModified()));
    connect(m_worksheet, SIGNAL(showHelp(const QString&)), h, SIGNAL(showHelp(const QString&)));
    connect(m_worksheet, SIGNAL(sessionChanged()), this, SLOT(worksheetSessionChanged()));

    // notify the part that this is our internal widget
    setWidget(m_worksheet);

    // create our actions
    KStandardAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    m_save = KStandardAction::save(this, SLOT(save()), actionCollection());

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

    m_tabcompletion=new KToggleAction(i18n("Tab Completion"), actionCollection());
    m_tabcompletion->setChecked(Settings::self()->tabCompletionDefault());
    actionCollection()->addAction("enable_tabcompletion", m_tabcompletion);
    connect(m_tabcompletion, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableTabCompletion(bool)));

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

    KAction* insertEntry=new KAction(i18n("Insert Entry"), actionCollection());
    insertEntry->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_entry",  insertEntry);
    connect(insertEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertEntry()));

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


    // set our XML-UI resource file
    setXMLFile("mathematik_part.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

    worksheetSessionChanged();
}

MathematiKPart::~MathematiKPart()
{
}

void MathematiKPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    m_worksheet->setReadOnly(!rw);

    ReadWritePart::setReadWrite(rw);
}

void MathematiKPart::setModified(bool modified)
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

KAboutData *MathematiKPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the
    // Makefile)
    KAboutData *aboutData = new KAboutData("mathematikpart", 0, ki18n("MathematiKPart"), "0.1");
    aboutData->addAuthor(ki18n("Alexander Rieder"), KLocalizedString(), "alexanderrieder@gmail.com");
    return aboutData;
}

bool MathematiKPart::openFile()
{
    m_worksheet->load(localFilePath());

    // just for fun, set the status bar
    //emit setStatusBarText( m_url.prettyUrl() );

    updateCaption();

    return true;
}

bool MathematiKPart::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    kDebug()<<"saving to: "<<url();
    if (url().isEmpty())
        fileSaveAs();
    else
    {
        if(url().fileName().endsWith(".mws"))
            m_worksheet->save( localFilePath() );
        else
            m_worksheet->savePlain( localFilePath());

    }
    setModified(false);

    return true;
}

void MathematiKPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString filter=i18n("*.mws|MathematiK Worksheet");

    //if the backend supports scripts, also append their scriptFile endings to the filter
    MathematiK::Backend * const backend=m_worksheet->session()->backend();
    if (backend->extensions().contains("ScriptExtension"))
    {
        MathematiK::ScriptExtension* e=dynamic_cast<MathematiK::ScriptExtension*>(backend->extension("ScriptExtension"));
        filter+='\n'+e->scriptFileFilter();
    }

    QString file_name = KFileDialog::getSaveFileName(KUrl(), filter, widget());
    if (file_name.isEmpty() == false)
        saveAs(file_name);

    updateCaption();
}

void MathematiKPart::guiActivateEvent( KParts::GUIActivateEvent * event )
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

void MathematiKPart::evaluateOrInterrupt()
{
    kDebug()<<"evalorinterrupt";
    if(m_worksheet->isRunning())
        m_worksheet->interrupt();
    else
        m_worksheet->evaluate();
}
void MathematiKPart::restartBackend()
{
    m_worksheet->session()->logout();
    m_worksheet->session()->login();
}

void MathematiKPart::worksheetStatusChanged(MathematiK::Session::Status status)
{
    kDebug()<<"wsStatusChange"<<status;
    if(status==MathematiK::Session::Running)
    {
        m_evaluate->setText(i18n("Interrupt"));
        m_evaluate->setIcon(KIcon("dialog-close"));

        emit setStatusBarText(i18n("Calcluating..."));
    }else
    {
        m_evaluate->setText(i18n("Evaluate Worksheet"));
        m_evaluate->setIcon(KIcon("system-run"));

        emit setStatusBarText(i18n("Ready"));
    }
}

void MathematiKPart::worksheetSessionChanged()
{
    connect(m_worksheet->session(), SIGNAL(statusChanged(MathematiK::Session::Status)), this, SLOT(worksheetStatusChanged(MathematiK::Session::Status)));
    connect(m_worksheet->session(), SIGNAL(ready()),this, SLOT(initialized()));

    loadAssistants();
    adjustGuiToSession();

    if(!m_initProgressDlg)
    {
        m_initProgressDlg=new KProgressDialog(widget(), i18n("MathematiK"), i18n("Initializing Session"));
        m_initProgressDlg->setMinimumDuration(500);
        m_initProgressDlg->progressBar()->setRange(0, 0);
    }
}

void MathematiKPart::initialized()
{
    m_worksheet->setEnabled(true);
    emit setStatusBarText(i18n("Initialization complete"));

    if(m_initProgressDlg)
    {
        m_initProgressDlg->deleteLater();
        m_initProgressDlg=0;
    }
    updateCaption();
}

void MathematiKPart::enableTypesetting(bool enable)
{
    m_worksheet->session()->setTypesettingEnabled(enable);
}

void MathematiKPart::showBackendHelp()
{
    kDebug()<<"showing backends help";
    MathematiK::Backend* backend=m_worksheet->session()->backend();
    KUrl url=backend->helpUrl();
    kDebug()<<"launching url "<<url;
    new KRun(url, widget());
}

Worksheet* MathematiKPart::worksheet()
{
    return m_worksheet;
}

void MathematiKPart::updateCaption()
{
    QString filename=url().fileName();
    //strip away the extension
    filename=filename.left(filename.lastIndexOf('.'));

    if (filename.isEmpty())
        filename=i18n("Unnamed");

    emit setCaption(i18n("%1: %2", m_worksheet->session()->backend()->name(), filename));
}

void MathematiKPart::loadAssistants()
{
    kDebug()<<"loading assistants...";

    KService::List services;
    KServiceTypeTrader* trader = KServiceTypeTrader::self();

    services = trader->query("MathematiK/Assistant");

    foreach (KService::Ptr service,   services)
    {
        QString error;

        kDebug()<<"found service"<<service->name();
        MathematiK::Assistant* assistant=service->createInstance<MathematiK::Assistant>(this,  QVariantList(),   &error);
        if (assistant==0)
        {
            kDebug()<<"error loading assistant"<<service->name()<<":  "<<error;
            continue;
        }

        kDebug()<<"created it";
        MathematiK::Backend* backend=worksheet()->session()->backend();
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
            assistant->deleteLater();
        }
    }


}

void MathematiKPart::runAssistant()
{
    MathematiK::Assistant* a=qobject_cast<MathematiK::Assistant*>(sender());
    QStringList cmds=a->run(widget());
    kDebug()<<cmds;
    if(!cmds.isEmpty())
        m_worksheet->appendEntry(cmds.join("\n"));
}

void MathematiKPart::adjustGuiToSession()
{
    m_typeset->setVisible(m_worksheet->session()->backend()->capabilities().testFlag(MathematiK::Backend::LaTexOutput));
    m_tabcompletion->setVisible(m_worksheet->session()->backend()->capabilities().testFlag(MathematiK::Backend::TabCompletion));

    //this is 0 on the first call
    if(m_showBackendHelp)
        m_showBackendHelp->setText(i18n("Show %1 Help", m_worksheet->session()->backend()->name()));
}

void MathematiKPart::publishWorksheet()
{
    int ret = KMessageBox::questionYesNo(widget(),
                                         i18n("Do you want to upload current Worksheet to public web server ?"),
                                         i18n("Question - MathematiK"));
    if (ret != KMessageBox::Yes) return;

    if (isModified()||url().isEmpty())
    {
        ret = KMessageBox::warningContinueCancel(widget(),
                                                 i18n("The Worksheet is not saved. You should save it before uploading."),
                                                 i18n("Warning - MathematiK"),  KStandardGuiItem::save(),  KStandardGuiItem::cancel());
        if (ret != KMessageBox::Continue) return;
        if (!saveFile()) return;
    }

    kDebug()<<"uploading file "<<url();

    KNS::Engine engine(widget());
    engine.init("mathematik.knsrc");
    KNS::Entry *entry = engine.uploadDialogModal(url().toLocalFile());

    if(entry)
    {
        KMessageBox::information(widget(), i18n("Uploading successful"), i18n("MathematiK"));
    }else
    {
        KMessageBox::error(widget(), i18n("Error uploading File %1", url().toLocalFile()), i18n("Error - MathematiK"));
    }
}

void MathematiKPart::print()
{
    QPrinter printer;
    QPointer<QPrintDialog> dialog = new QPrintDialog(&printer,  widget());

    if (m_worksheet->textCursor().hasSelection())
        dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);

    if (dialog->exec() == QDialog::Accepted)
        m_worksheet->print(&printer);

    delete dialog;
}

void MathematiKPart::showScriptEditor(bool show)
{
    MathematiK::Backend* backend=m_worksheet->session()->backend();
    if(!backend->extensions().contains("ScriptExtension"))
    {
        KMessageBox::error(widget(), i18n("This backend doesn't support scripts"), i18n("Error - MathematiK"));
        return;
    }

    if(show)
    {
        MathematiK::ScriptExtension* scriptE=dynamic_cast<MathematiK::ScriptExtension*>(backend->extension("ScriptExtension"));
        m_scriptEditor=new ScriptEditorWidget(scriptE->scriptFileFilter());
        connect(m_scriptEditor, SIGNAL(runScript(const QString&)), this, SLOT(runScript(const QString&)));
        m_scriptEditor->show();
    }else
    {
        m_scriptEditor->deleteLater();
        m_scriptEditor=0;
    }
}

void MathematiKPart::runScript(const QString& file)
{
    MathematiK::Backend* backend=m_worksheet->session()->backend();
    if(!backend->extensions().contains("ScriptExtension"))
    {
        KMessageBox::error(widget(), i18n("This backend doesn't support scripts"), i18n("Error - MathematiK"));
        return;
    }

    MathematiK::ScriptExtension* scriptE=dynamic_cast<MathematiK::ScriptExtension*>(backend->extension("ScriptExtension"));
    m_worksheet->appendEntry(scriptE->runExternalScript(file));
}
