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

#include <config-cantor.h>

#include <KLocalizedString>
#include <QIcon>
#include <KParts/Event>
#include <KParts/GUIActivateEvent>
#include <KPluginFactory>
#include <KAboutData>

#include <QAction>
#include <KActionCollection>
#include <QFileDialog>
#include <KStandardAction>
#include <KZip>
#include <KToggleAction>
#include <KService>
#include <KServiceTypeTrader>
#include <KRun>
#include <QProgressDialog>
#include <KMessageBox>
#include <KNS3/UploadDialog>

#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QTextEdit>
#include <QTimer>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QVBoxLayout>

#include "worksheet.h"
#include "worksheetview.h"
#include "searchbar.h"
#include "scripteditorwidget.h"
#include "lib/backend.h"
#include "lib/extension.h"
#include "lib/assistant.h"
#include "lib/panelpluginhandler.h"
#include "lib/panelplugin.h"
#include "lib/worksheetaccess.h"

#include "settings.h"


//A concrete implementation of the WorksheetAccesssInterface
class WorksheetAccessInterfaceImpl : public Cantor::WorksheetAccessInterface
{
    //Q_OBJECT
  public:
    WorksheetAccessInterfaceImpl(QObject* parent, Worksheet* worksheet) :   WorksheetAccessInterface(parent),  m_worksheet(worksheet)
    {
        qDebug()<<"new worksheetaccess interface";
        connect(worksheet, SIGNAL(sessionChanged()), this, SIGNAL(sessionChanged()));

    }

    ~WorksheetAccessInterfaceImpl() override
    {

    }

    QByteArray saveWorksheetToByteArray() Q_DECL_OVERRIDE
    {
        return m_worksheet->saveToByteArray();
    }

    void loadWorksheetFromByteArray(QByteArray* data) Q_DECL_OVERRIDE
    {
        m_worksheet->load(data);
    }

    Cantor::Session* session() Q_DECL_OVERRIDE
    {
        return m_worksheet->session();
    }
//  public Q_SLOTS:
  void evaluate() Q_DECL_OVERRIDE
    {
        m_worksheet->evaluate();
    }

    void interrupt() Q_DECL_OVERRIDE
    {
        m_worksheet->interrupt();
    }

  private:
    Worksheet* m_worksheet;
};

CantorPart::CantorPart( QWidget *parentWidget, QObject *parent, const QVariantList & args ): KParts::ReadWritePart(parent),
    m_searchBar(nullptr),
    m_initProgressDlg(nullptr),
    m_showProgressDlg(true),
    m_showBackendHelp(nullptr),
    m_statusBarBlocked(false)
{
    qDebug()<<"Created a CantorPart";

    m_panelHandler=new Cantor::PanelPluginHandler(this);
    connect(m_panelHandler, SIGNAL(pluginsChanged()), this, SLOT(pluginsChanged()));

    QString backendName;
    if(args.isEmpty())
        backendName=QLatin1String("null");
    else
        backendName=args.first().toString();

    foreach(const QVariant& arg, args)
    {
        if (arg.toString() == QLatin1String("--noprogress") )
        {
            qWarning()<<"not showing the progress bar by request";
            m_showProgressDlg=false;
        }
    }

    Cantor::Backend* b=Cantor::Backend::createBackend(backendName);
    if(!b)
    {
        KMessageBox::error(parentWidget, i18n("Backend %1 is not installed", backendName), i18n("Error - Cantor"));
        setWidget(new QWidget(parentWidget));
        //fake being modified so the shell won't try to reuse this part
        ReadWritePart::setModified(true);
        return;
    }

    qDebug()<<"Backend "<<b->name()<<" offers extensions: "<<b->extensions();

    QWidget* widget = new QWidget(parentWidget);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    m_worksheet=new Worksheet(b, widget);
    m_worksheetview=new WorksheetView(m_worksheet, widget);
    m_worksheetview->setEnabled(false); //disable input until the session has successfully logged in and emits the ready signal
    connect(m_worksheet, SIGNAL(modified()), this, SLOT(setModified()));
    connect(m_worksheet, SIGNAL(showHelp(const QString&)), this, SIGNAL(showHelp(const QString&)));
    connect(m_worksheet, SIGNAL(sessionChanged()), this, SLOT(worksheetSessionChanged()));
    connect(actionCollection(), SIGNAL(inserted(QAction*)), m_worksheet,
            SLOT(registerShortcut(QAction*)));

    layout->addWidget(m_worksheetview);
    // notify the part that this is our internal widget
    setWidget(widget);

    Cantor::WorksheetAccessInterface* iface=new WorksheetAccessInterfaceImpl(this, m_worksheet);
    Q_UNUSED(iface);

    // create our actions
    m_worksheet->createActions(actionCollection());

    KStandardAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    m_save = KStandardAction::save(this, SLOT(save()), actionCollection());
    m_save->setPriority(QAction::LowPriority);

    QAction * savePlain=new QAction(i18n("Save Plain Text"), actionCollection());
    actionCollection()->addAction(QLatin1String("file_save_plain"), savePlain);
    savePlain->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    connect(savePlain, SIGNAL(triggered()), this, SLOT(fileSavePlain()));

    QAction * undo=KStandardAction::undo(m_worksheet, SIGNAL(undo()),
                                        actionCollection());
    undo->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(undoAvailable(bool)),
            undo, SLOT(setEnabled(bool)));
    QAction * redo=KStandardAction::redo(m_worksheet, SIGNAL(redo()),
                                        actionCollection());
    redo->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(redoAvailable(bool)),
            redo, SLOT(setEnabled(bool)));

    QAction * cut=KStandardAction::cut(m_worksheet, SIGNAL(cut()),
                                      actionCollection());
    cut->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(cutAvailable(bool)),
            cut, SLOT(setEnabled(bool)));
    QAction * copy=KStandardAction::copy(m_worksheet, SIGNAL(copy()),
                                        actionCollection());
    copy->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(copyAvailable(bool)),
            copy, SLOT(setEnabled(bool)));
    QAction * paste=KStandardAction::paste(m_worksheet, SIGNAL(paste()),
                                          actionCollection());
    paste->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(pasteAvailable(bool)),
            paste, SLOT(setEnabled(bool)));

    QAction * find=KStandardAction::find(this, SLOT(showSearchBar()),
                                        actionCollection());
    find->setPriority(QAction::LowPriority);

    QAction * replace=KStandardAction::replace(this, SLOT(showExtendedSearchBar()),
                                              actionCollection());
    replace->setPriority(QAction::LowPriority);

    m_findNext = KStandardAction::findNext(this, SLOT(findNext()),
                                           actionCollection());
    m_findNext->setEnabled(false);
    m_findPrev = KStandardAction::findPrev(this, SLOT(findPrev()),
                                           actionCollection());
    m_findPrev->setEnabled(false);

    QAction * latexExport=new QAction(i18n("Export to LaTeX"), actionCollection());
    actionCollection()->addAction(QLatin1String("file_export_latex"), latexExport);
    latexExport->setIcon(QIcon::fromTheme(QLatin1String("document-export")));
    connect(latexExport, SIGNAL(triggered()), this, SLOT(exportToLatex()));

    QAction * print = KStandardAction::print(this, SLOT(print()), actionCollection());
    print->setPriority(QAction::LowPriority);

    QAction * printPreview = KStandardAction::printPreview(this, SLOT(printPreview()), actionCollection());
    printPreview->setPriority(QAction::LowPriority);

    KStandardAction::zoomIn(m_worksheetview, SLOT(zoomIn()), actionCollection());
    KStandardAction::zoomOut(m_worksheetview, SLOT(zoomOut()), actionCollection());

    m_evaluate=new QAction(i18n("Evaluate Worksheet"), actionCollection());
    actionCollection()->addAction(QLatin1String("evaluate_worksheet"), m_evaluate);
    m_evaluate->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    m_evaluate->setShortcut(Qt::CTRL+Qt::Key_E);
    connect(m_evaluate, SIGNAL(triggered()), this, SLOT(evaluateOrInterrupt()));

    m_typeset=new KToggleAction(i18n("Typeset using LaTeX"), actionCollection());
    m_typeset->setChecked(Settings::self()->typesetDefault());
    actionCollection()->addAction(QLatin1String("enable_typesetting"), m_typeset);
    connect(m_typeset, SIGNAL(toggled(bool)), this, SLOT(enableTypesetting(bool)));

    m_highlight=new KToggleAction(i18n("Syntax Highlighting"), actionCollection());
    m_highlight->setChecked(Settings::self()->highlightDefault());
    actionCollection()->addAction(QLatin1String("enable_highlighting"), m_highlight);
    connect(m_highlight, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableHighlighting(bool)));

    m_completion=new KToggleAction(i18n("Completion"), actionCollection());
    m_completion->setChecked(Settings::self()->completionDefault());
    actionCollection()->addAction(QLatin1String("enable_completion"), m_completion);
    connect(m_completion, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableCompletion(bool)));

    m_exprNumbering=new KToggleAction(i18n("Line Numbers"), actionCollection());
    m_exprNumbering->setChecked(Settings::self()->expressionNumberingDefault());
    actionCollection()->addAction(QLatin1String("enable_expression_numbers"), m_exprNumbering);
    connect(m_exprNumbering, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableExpressionNumbering(bool)));

    m_animateWorksheet=new KToggleAction(i18n("Animate Worksheet"), actionCollection());
    m_animateWorksheet->setChecked(Settings::self()->animationDefault());
    actionCollection()->addAction(QLatin1String("enable_animations"), m_animateWorksheet);
    connect(m_animateWorksheet, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableAnimations(bool)));

    QAction * restart=new QAction(i18n("Restart Backend"), actionCollection());
    actionCollection()->addAction(QLatin1String("restart_backend"), restart);
    restart->setIcon(QIcon::fromTheme(QLatin1String("system-reboot")));
    connect(restart, SIGNAL(triggered()), this, SLOT(restartBackend()));

    QAction * evaluateCurrent=new QAction(i18n("Evaluate Entry"), actionCollection());
    actionCollection()->addAction(QLatin1String("evaluate_current"),  evaluateCurrent);
    actionCollection()->setDefaultShortcut(evaluateCurrent, Qt::SHIFT + Qt::Key_Return);
    connect(evaluateCurrent, SIGNAL(triggered()), m_worksheet, SLOT(evaluateCurrentEntry()));

    QAction * insertCommandEntry=new QAction(i18n("Insert Command Entry"), actionCollection());
    actionCollection()->addAction(QLatin1String("insert_command_entry"),  insertCommandEntry);
    actionCollection()->setDefaultShortcut(insertCommandEntry, Qt::CTRL + Qt::Key_Return);
    connect(insertCommandEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertCommandEntry()));

    QAction * insertTextEntry=new QAction(i18n("Insert Text Entry"), actionCollection());
    actionCollection()->addAction(QLatin1String("insert_text_entry"),  insertTextEntry);
    connect(insertTextEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertTextEntry()));

    QAction * insertLatexEntry=new QAction(i18n("Insert Latex Entry"), actionCollection());
    actionCollection()->addAction(QLatin1String("insert_latex_entry"),  insertLatexEntry);
    connect(insertLatexEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertLatexEntry()));

    QAction * insertPageBreakEntry=new QAction(i18n("Insert Page Break"), actionCollection());
    actionCollection()->addAction(QLatin1String("insert_page_break_entry"), insertPageBreakEntry);
    connect(insertPageBreakEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertPageBreakEntry()));

    QAction * insertImageEntry=new QAction(i18n("Insert Image"), actionCollection());
    actionCollection()->addAction(QLatin1String("insert_image_entry"), insertImageEntry);
    connect(insertImageEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertImageEntry()));


    /*
    QAction * insertCommandEntryBefore=new QAction(i18n("Insert Command Entry Before"), actionCollection());
    //insertCommandEntryBefore->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_command_entry_before",  insertCommandEntryBefore);
    connect(insertCommandEntryBefore, SIGNAL(triggered()), m_worksheet, SLOT(insertCommandEntryBefore()));

    QAction * insertTextEntryBefore=new QAction(i18n("Insert Text Entry Before"), actionCollection());
    //insertTextEntryBefore->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_text_entry_before",  insertTextEntryBefore);
    connect(insertTextEntryBefore, SIGNAL(triggered()), m_worksheet, SLOT(insertTextEntryBefore()));

    QAction * insertPageBreakEntryBefore=new QAction(i18n("Insert Page Break Before"), actionCollection());
    actionCollection()->addAction("insert_page_break_entry_before", insertPageBreakEntryBefore);
    connect(insertPageBreakEntryBefore, SIGNAL(triggered()), m_worksheet, SLOT(insertPageBreakEntryBefore()));

    QAction * insertImageEntryBefore=new QAction(i18n("Insert Image Entry Before"), actionCollection());
    //insertTextEntryBefore->setShortcut(Qt::CTRL + Qt::Key_Return);
    actionCollection()->addAction("insert_image_entry_before",  insertImageEntryBefore);
    connect(insertImageEntryBefore, SIGNAL(triggered()), m_worksheet, SLOT(insertImageEntryBefore()));
    */

    QAction * removeCurrent=new QAction(i18n("Remove current Entry"), actionCollection());
    actionCollection()->addAction(QLatin1String("remove_current"), removeCurrent);
    actionCollection()->setDefaultShortcut(removeCurrent, Qt::ShiftModifier + Qt::Key_Delete);
    connect(removeCurrent, SIGNAL(triggered()), m_worksheet, SLOT(removeCurrentEntry()));

    m_showBackendHelp=new QAction(i18n("Show %1 Help", b->name()) , actionCollection());
    m_showBackendHelp->setIcon(QIcon::fromTheme(QLatin1String("help-contents")));
    actionCollection()->addAction(QLatin1String("backend_help"), m_showBackendHelp);
    connect(m_showBackendHelp, SIGNAL(triggered()), this, SLOT(showBackendHelp()));

    QAction * publishWorksheet=new QAction(i18n("Publish Worksheet"), actionCollection());
    publishWorksheet->setIcon(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")));
    actionCollection()->addAction(QLatin1String("file_publish_worksheet"), publishWorksheet);
    connect(publishWorksheet, SIGNAL(triggered()), this, SLOT(publishWorksheet()));

    KToggleAction* showEditor=new KToggleAction(i18n("Show Script Editor"), actionCollection());
    showEditor->setChecked(false);
    actionCollection()->addAction(QLatin1String("show_editor"), showEditor);
    connect(showEditor, SIGNAL(toggled(bool)), this, SLOT(showScriptEditor(bool)));
    showEditor->setEnabled(b->extensions().contains(QLatin1String("ScriptExtension")));

    QAction * showCompletion=new QAction(i18n("Show Completion"), actionCollection());
    actionCollection()->addAction(QLatin1String("show_completion"), showCompletion);
    QList<QKeySequence> showCompletionShortcuts;
    showCompletionShortcuts << Qt::Key_Tab << Qt::CTRL + Qt::Key_Space;
    actionCollection()->setDefaultShortcuts(showCompletion, showCompletionShortcuts);
    connect(showCompletion, SIGNAL(triggered()), m_worksheet, SLOT(showCompletion()));

    // set our XML-UI resource file
    setXMLFile(QLatin1String("cantor_part.rc"));

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
    if (m_searchBar)
        delete m_searchBar;
}

void CantorPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
    m_worksheetview->setInteractive(rw);

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

KAboutData& CantorPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the
    // Makefile)

    static KAboutData about(QLatin1String("cantorpart"),
                     QLatin1String("Cantor"),
                     QLatin1String(CANTOR_VERSION),
                     i18n("CantorPart"),
                     KAboutLicense::GPL,
                     i18n("(C) 2009-2015 Alexander Rieder"),
                     QString(),
                     QLatin1String("http://edu.kde.org/cantor"));

    about.addAuthor( i18n("Alexander Rieder"), QString(), QLatin1String("alexanderrieder@gmail.com") );
    return about;
}

bool CantorPart::openFile()
{
    //don't crash if for some reason the worksheet is invalid
    if(m_worksheet==nullptr)
    {
        qWarning()<<"trying to open in an invalid cantor part";
        return false;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QElapsedTimer timer;
	timer.start();
    m_worksheet->load(localFilePath());
    qDebug()<< "Worksheet successfully loaded in " <<  (float)timer.elapsed()/1000 << " seconds).";
    QApplication::restoreOverrideCursor();

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

    qDebug()<<"saving to: "<<url();
    if (url().isEmpty())
        fileSaveAs();
    else
        m_worksheet->save( localFilePath() );
    setModified(false);

    return true;
}

void CantorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected
    QString worksheetFilter = i18n("Cantor Worksheet (*.cws)");
    QString filter = worksheetFilter;

    //if the backend supports scripts, also append their scriptFile endings to the filter
    Cantor::Backend * const backend=m_worksheet->session()->backend();
    if (backend->extensions().contains(QLatin1String("ScriptExtension")))
    {
        Cantor::ScriptExtension* e=dynamic_cast<Cantor::ScriptExtension*>(backend->extension(QLatin1String("ScriptExtension")));
        filter+=QLatin1Char('\n')+e->scriptFileFilter();
    }

    QString selectedFilter;
    QString file_name = QFileDialog::getSaveFileName(widget(), i18n("Save as"), QString(), filter, &selectedFilter);
    if (file_name.isEmpty())
        return;

    //depending on user's selection, save as a worksheet or as a plain script file
    if (selectedFilter == worksheetFilter)
    {
        if (!file_name.endsWith(QLatin1String(".cws")))
            file_name += QLatin1String(".cws");
        saveAs(QUrl::fromLocalFile(file_name));
    }
    else
        m_worksheet->savePlain(file_name);

    updateCaption();
}

void CantorPart::fileSavePlain()
{
    QString file_name = QFileDialog::getSaveFileName(widget(), i18n("Save"), QString(), QString());
    if (!file_name.isEmpty())
        m_worksheet->savePlain(file_name);
}

void CantorPart::exportToLatex()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString filter=i18n("*.tex|LaTeX Document");

    QString file_name = QFileDialog::getSaveFileName(widget(), i18n("Export to LaTeX"), QString(), QString());

    if (file_name.isEmpty() == false)
        m_worksheet->saveLatex(file_name);
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
    qDebug()<<"evalorinterrupt";
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
    qDebug()<<"wsStatusChange"<<status;
    if(status==Cantor::Session::Running)
    {
        m_evaluate->setText(i18n("Interrupt"));
        m_evaluate->setShortcut(Qt::CTRL+Qt::Key_I);
        m_evaluate->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));

        setStatusMessage(i18n("Calculating..."));
    }else
    {
        m_evaluate->setText(i18n("Evaluate Worksheet"));
        m_evaluate->setShortcut(Qt::CTRL+Qt::Key_E);
        m_evaluate->setIcon(QIcon::fromTheme(QLatin1String("system-run")));

        setStatusMessage(i18n("Ready"));
    }
}

void CantorPart::showSessionError(const QString& message)
{
    qDebug()<<"Error: "<<message;
    initialized();
    showImportantStatusMessage(i18n("Session Error: %1", message));
}

void CantorPart::worksheetSessionChanged()
{
    connect(m_worksheet->session(), SIGNAL(statusChanged(Cantor::Session::Status)), this, SLOT(worksheetStatusChanged(Cantor::Session::Status)));
    connect(m_worksheet->session(), SIGNAL(loginStarted()),this, SLOT(worksheetSessionLoginStarted()));
    connect(m_worksheet->session(), SIGNAL(loginDone()),this, SLOT(worksheetSessionLoginDone()));
    connect(m_worksheet->session(), SIGNAL(error(const QString&)), this, SLOT(showSessionError(const QString&)));

    loadAssistants();
    m_panelHandler->setSession(m_worksheet->session());
    adjustGuiToSession();
    initialized();
}

void CantorPart::initialized()
{
    if (m_worksheet->isEmpty())
        m_worksheet->appendCommandEntry();

    m_worksheetview->setEnabled(true);
    m_worksheetview->setFocus();
    setStatusMessage(i18n("Initialization complete"));
    updateCaption();
}

void CantorPart::worksheetSessionLoginStarted() {
    setStatusMessage(i18n("Initializing..."));
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void CantorPart::worksheetSessionLoginDone() {
    setStatusMessage(i18n("Ready"));
    QApplication::restoreOverrideCursor();
}

void CantorPart::enableTypesetting(bool enable)
{
    m_worksheet->session()->setTypesettingEnabled(enable);
}

void CantorPart::showBackendHelp()
{
    qDebug()<<"showing backends help";
    Cantor::Backend* backend=m_worksheet->session()->backend();
    QUrl url = backend->helpUrl();
    qDebug()<<"launching url "<<url;
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
    filename=filename.left(filename.lastIndexOf(QLatin1Char('.')));

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
    qDebug()<<"loading assistants...";

    QStringList assistantDirs;
    foreach(const QString &dir, QCoreApplication::libraryPaths()){
        assistantDirs << dir + QDir::separator() + QLatin1String("cantor/assistants");
    }

    QPluginLoader loader;
    foreach(const QString &dir, assistantDirs){

        qDebug() << "dir: " << dir;
        QStringList assistants;
        QDir assistantDir = QDir(dir);

        assistants = assistantDir.entryList();

        foreach (const QString &assistant, assistants){
            if (assistant==QLatin1String(".") || assistant==QLatin1String(".."))
                continue;

            loader.setFileName(dir + QDir::separator() + assistant);

            if (!loader.load()){
                qDebug() << "Error while loading assistant: " << assistant;
                continue;
            }

            KPluginFactory* factory = KPluginLoader(loader.fileName()).factory();
            Cantor::Assistant* plugin = factory->create<Cantor::Assistant>(this);

            Cantor::Backend* backend=worksheet()->session()->backend();

            KPluginMetaData info(loader);
            plugin->setPluginInfo(info);
            plugin->setBackend(backend);

            qDebug()<<"plugin "<<info.name()<<" requires "<<plugin->requiredExtensions();
            bool supported=true;
            foreach(const QString& req, plugin->requiredExtensions())
                supported=supported && backend->extensions().contains(req);

            qDebug()<<"plugin "<<info.name()<<" is "<<(supported ? "":" not ")<<" supported by "<<backend->name();

            if(supported)
            {
                plugin->initActions();
                connect(plugin, SIGNAL(requested()), this, SLOT(runAssistant()));
            }else
            {
                removeChildClient(plugin);
                plugin->deleteLater();
            }
        }
    }
}

void CantorPart::runAssistant()
{
    Cantor::Assistant* a=qobject_cast<Cantor::Assistant*>(sender());
    QStringList cmds=a->run(widget());
    qDebug()<<cmds;
    if(!cmds.isEmpty())
        runCommand(cmds.join(QLatin1String("\n")));
}

void CantorPart::runCommand(const QString& cmd)
{
    m_worksheet->appendCommandEntry(cmd);
}

void CantorPart::showSearchBar()
{
    if (!m_searchBar) {
        m_searchBar = new SearchBar(widget(), m_worksheet);
        widget()->layout()->addWidget(m_searchBar);
        connect(m_searchBar, SIGNAL(destroyed(QObject*)),
                this, SLOT(searchBarDeleted()));
    }

    m_findNext->setEnabled(true);
    m_findPrev->setEnabled(true);

    m_searchBar->showStandard();
    m_searchBar->setFocus();
}

void CantorPart::showExtendedSearchBar()
{
    if (!m_searchBar) {
        m_searchBar = new SearchBar(widget(), m_worksheet);
        widget()->layout()->addWidget(m_searchBar);
        connect(m_searchBar, SIGNAL(destroyed(QObject*)),
                this, SLOT(searchBarDeleted()));
    }

    m_findNext->setEnabled(true);
    m_findPrev->setEnabled(true);

    m_searchBar->showExtended();
    m_searchBar->setFocus();
}

void CantorPart::findNext()
{
    if (m_searchBar)
        m_searchBar->next();
}

void CantorPart::findPrev()
{
    if (m_searchBar)
        m_searchBar->prev();
}

void CantorPart::searchBarDeleted()
{
    m_searchBar = nullptr;
    m_findNext->setEnabled(false);
    m_findPrev->setEnabled(false);
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

    qDebug()<<"uploading file "<<url();

    // upload
    //HACK: use different .knsrc files for each category
    //remove this once KNS3 gains the ability to select category
    KNS3::UploadDialog dialog(QString::fromLatin1("cantor_%1.knsrc").arg(m_worksheet->session()->backend()->id().toLower()), widget());
    dialog.setUploadFile(url());
    dialog.exec();
}

void CantorPart::print()
{
    QPrinter printer;
    QPointer<QPrintDialog> dialog = new QPrintDialog(&printer,  widget());

    // TODO: Re-enable print selection
    //if (m_worksheet->textCursor().hasSelection())
    //    dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);

    if (dialog->exec() == QDialog::Accepted)
        m_worksheet->print(&printer);

    delete dialog;
}

void CantorPart::printPreview()
{
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(widget());
    connect(dialog, SIGNAL(paintRequested(QPrinter*)), m_worksheet, SLOT(print(QPrinter*)));
    dialog->exec();
}

void CantorPart::showScriptEditor(bool show)
{
    if(show)
    {
        if (m_scriptEditor)
        {
            return;
        }
        Cantor::ScriptExtension* scriptE=dynamic_cast<Cantor::ScriptExtension*>(m_worksheet->session()->backend()->extension(QLatin1String("ScriptExtension")));
        if (!scriptE)
        {
            return;
        }
        m_scriptEditor=new ScriptEditorWidget(scriptE->scriptFileFilter(), scriptE->highlightingMode(), widget()->window());
        connect(m_scriptEditor, SIGNAL(runScript(const QString&)), this, SLOT(runScript(const QString&)));
        connect(m_scriptEditor, SIGNAL(destroyed()), this, SLOT(scriptEditorClosed()));
        m_scriptEditor->show();
    }else
    {
        m_scriptEditor->deleteLater();
    }
}

void CantorPart::scriptEditorClosed()
{
    QAction* showEditor = actionCollection()->action(QLatin1String("show_editor"));
    if (showEditor)
    {
        showEditor->setChecked(false);
    }
}

void CantorPart::runScript(const QString& file)
{
    Cantor::Backend* backend=m_worksheet->session()->backend();
    if(!backend->extensions().contains(QLatin1String("ScriptExtension")))
    {
        KMessageBox::error(widget(), i18n("This backend does not support scripts."), i18n("Error - Cantor"));
        return;
    }

    Cantor::ScriptExtension* scriptE=dynamic_cast<Cantor::ScriptExtension*>(backend->extension(QLatin1String("ScriptExtension")));
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

K_PLUGIN_FACTORY_WITH_JSON(CantorPartFactory, "cantor_part.json", registerPlugin<CantorPart>();)
#include "cantor_part.moc"
