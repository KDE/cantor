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
#include "scripteditor/scripteditorwidget.h"
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

  public:
    WorksheetAccessInterfaceImpl(QObject* parent, Worksheet* worksheet) :   WorksheetAccessInterface(parent),  m_worksheet(worksheet)
    {
        qDebug()<<"new worksheetaccess interface";
        connect(worksheet, SIGNAL(modified()), this, SIGNAL(modified()));
    }

    ~WorksheetAccessInterfaceImpl() override = default;

    QByteArray saveWorksheetToByteArray() override
    {
        return m_worksheet->saveToByteArray();
    }

    void loadWorksheetFromByteArray(QByteArray* data) override
    {
        m_worksheet->load(data);
    }

    Cantor::Session* session() override
    {
        return m_worksheet->session();
    }

  void evaluate() override
    {
        m_worksheet->evaluate();
    }

    void interrupt() override
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
    m_statusBarBlocked(false),
    m_sessionStatusCounter(0)
{
    m_panelHandler=new Cantor::PanelPluginHandler(this);
    connect(m_panelHandler, SIGNAL(pluginsChanged()), this, SLOT(pluginsChanged()));

    QString backendName;
    if(args.isEmpty())
        backendName=QLatin1String("null");
    else
        backendName=args.first().toString();

    for (const QVariant& arg : args)
    {
        if (arg.toString() == QLatin1String("--noprogress") )
        {
            qWarning()<<"not showing the progress bar by request";
            m_showProgressDlg=false;
        }
    }

    Cantor::Backend* b=Cantor::Backend::getBackend(backendName);
    if(!b)
    {
        KMessageBox::error(parentWidget, i18n("Backend %1 is not installed", backendName), i18n("Error - Cantor"));
        setWidget(new QWidget(parentWidget));
        return;
    }

    qDebug()<<"Backend "<<b->name()<<" offers extensions: "<<b->extensions();


    auto* collection = actionCollection();

    //central widget
    QWidget* widget = new QWidget(parentWidget);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    m_worksheet=new Worksheet(b, widget);
    m_worksheetview=new WorksheetView(m_worksheet, widget);
    m_worksheetview->setEnabled(false); //disable input until the session has successfully logged in and emits the ready signal
    connect(m_worksheet, SIGNAL(modified()), this, SLOT(setModified()));
    connect(m_worksheet, SIGNAL(showHelp(QString)), this, SIGNAL(showHelp(QString)));
    connect(m_worksheet, SIGNAL(loaded()), this, SLOT(initialized()));
    connect(collection, SIGNAL(inserted(QAction*)), m_worksheet, SLOT(registerShortcut(QAction*)));
    layout->addWidget(m_worksheetview);
    setWidget(widget);

    //create WorksheetAccessInterface, used at the moment by LabPlot only to access Worksheet's API
    Cantor::WorksheetAccessInterface* iface = new WorksheetAccessInterfaceImpl(this, m_worksheet);
    Q_UNUSED(iface);

    //initialize actions
    m_worksheet->createActions(collection);

    KStandardAction::saveAs(this, SLOT(fileSaveAs()), collection);
    m_save = KStandardAction::save(this, SLOT(save()), collection);
    m_save->setPriority(QAction::LowPriority);

    QAction* savePlain = new QAction(i18n("Save Plain Text"), collection);
    collection->addAction(QLatin1String("file_save_plain"), savePlain);
    savePlain->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    connect(savePlain, SIGNAL(triggered()), this, SLOT(fileSavePlain()));

    QAction* undo = KStandardAction::undo(m_worksheet, SIGNAL(undo()), collection);
    undo->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)));

    QAction* redo = KStandardAction::redo(m_worksheet, SIGNAL(redo()), collection);
    redo->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(redoAvailable(bool)), redo, SLOT(setEnabled(bool)));

    QAction* cut = KStandardAction::cut(m_worksheet, SIGNAL(cut()), collection);
    cut->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(cutAvailable(bool)), cut, SLOT(setEnabled(bool)));

    QAction* copy = KStandardAction::copy(m_worksheet, SIGNAL(copy()), collection);
    copy->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(copyAvailable(bool)), copy, SLOT(setEnabled(bool)));

    QAction* paste = KStandardAction::paste(m_worksheet, SLOT(paste()), collection);
    paste->setPriority(QAction::LowPriority);
    connect(m_worksheet, SIGNAL(pasteAvailable(bool)), paste, SLOT(setEnabled(bool)));

    QAction* find = KStandardAction::find(this, SLOT(showSearchBar()), collection);
    find->setPriority(QAction::LowPriority);

    QAction* replace = KStandardAction::replace(this, SLOT(showExtendedSearchBar()), collection);
    replace->setPriority(QAction::LowPriority);

    m_findNext = KStandardAction::findNext(this, SLOT(findNext()), collection);
    m_findNext->setEnabled(false);

    m_findPrev = KStandardAction::findPrev(this, SLOT(findPrev()), collection);
    m_findPrev->setEnabled(false);

    QAction* latexExport = new QAction(i18n("Export to LaTeX"), collection);
    collection->addAction(QLatin1String("file_export_latex"), latexExport);
    latexExport->setIcon(QIcon::fromTheme(QLatin1String("document-export")));
    connect(latexExport, SIGNAL(triggered()), this, SLOT(exportToLatex()));

    QAction* print = KStandardAction::print(this, SLOT(print()), collection);
    print->setPriority(QAction::LowPriority);

    QAction* printPreview = KStandardAction::printPreview(this, SLOT(printPreview()), collection);
    printPreview->setPriority(QAction::LowPriority);

    KStandardAction::zoomIn(m_worksheetview, SLOT(zoomIn()), collection);
    KStandardAction::zoomOut(m_worksheetview, SLOT(zoomOut()), collection);

    m_evaluate = new QAction(i18n("Evaluate Worksheet"), collection);
    collection->addAction(QLatin1String("evaluate_worksheet"), m_evaluate);
    m_evaluate->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    collection->setDefaultShortcut(m_evaluate, Qt::CTRL+Qt::Key_E);
    connect(m_evaluate, SIGNAL(triggered()), this, SLOT(evaluateOrInterrupt()));

    m_typeset = new KToggleAction(i18n("Typeset using LaTeX"), collection);
    m_typeset->setChecked(Settings::self()->typesetDefault());
    // Disable until login, because we use session command for this action
    m_typeset->setEnabled(false);
    collection->addAction(QLatin1String("enable_typesetting"), m_typeset);
    connect(m_typeset, SIGNAL(toggled(bool)), this, SLOT(enableTypesetting(bool)));

    m_highlight = new KToggleAction(i18n("Syntax Highlighting"), collection);
    m_highlight->setChecked(Settings::self()->highlightDefault());
    collection->addAction(QLatin1String("enable_highlighting"), m_highlight);
    connect(m_highlight, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableHighlighting(bool)));

    m_completion = new KToggleAction(i18n("Completion"), collection);
    m_completion->setChecked(Settings::self()->completionDefault());
    collection->addAction(QLatin1String("enable_completion"), m_completion);
    connect(m_completion, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableCompletion(bool)));

    m_exprNumbering = new KToggleAction(i18n("Line Numbers"), collection);
    m_exprNumbering->setChecked(Settings::self()->expressionNumberingDefault());
    collection->addAction(QLatin1String("enable_expression_numbers"), m_exprNumbering);
    connect(m_exprNumbering, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableExpressionNumbering(bool)));

    m_animateWorksheet = new KToggleAction(i18n("Animate Worksheet"), collection);
    m_animateWorksheet->setChecked(Settings::self()->animationDefault());
    collection->addAction(QLatin1String("enable_animations"), m_animateWorksheet);
    connect(m_animateWorksheet, SIGNAL(toggled(bool)), m_worksheet, SLOT(enableAnimations(bool)));

    QAction* restart = new QAction(i18n("Restart Backend"), collection);
    collection->addAction(QLatin1String("restart_backend"), restart);
    restart->setIcon(QIcon::fromTheme(QLatin1String("system-reboot")));
    connect(restart, SIGNAL(triggered()), this, SLOT(restartBackend()));

    QAction* evaluateCurrent = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Evaluate Entry"), collection);
    collection->addAction(QLatin1String("evaluate_current"),  evaluateCurrent);
    collection->setDefaultShortcut(evaluateCurrent, Qt::SHIFT + Qt::Key_Return);
    connect(evaluateCurrent, SIGNAL(triggered()), m_worksheet, SLOT(evaluateCurrentEntry()));

    QAction* insertCommandEntry = new QAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Insert Command Entry"), collection);
    collection->addAction(QLatin1String("insert_command_entry"),  insertCommandEntry);
    collection->setDefaultShortcut(insertCommandEntry, Qt::CTRL + Qt::Key_Return);
    connect(insertCommandEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertCommandEntry()));

    QAction* insertTextEntry = new QAction(QIcon::fromTheme(QLatin1String("draw-text")), i18n("Insert Text Entry"), collection);
    collection->addAction(QLatin1String("insert_text_entry"),  insertTextEntry);
    connect(insertTextEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertTextEntry()));

#ifdef Discount_FOUND
    QAction* insertMarkdownEntry = new QAction(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Insert Markdown Entry"), collection);
    collection->addAction(QLatin1String("insert_markdown_entry"),  insertMarkdownEntry);
    connect(insertMarkdownEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertMarkdownEntry()));
#endif

    QAction* insertLatexEntry = new QAction(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("Insert Latex Entry"), collection);
    collection->addAction(QLatin1String("insert_latex_entry"),  insertLatexEntry);
    connect(insertLatexEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertLatexEntry()));

    QAction* insertPageBreakEntry = new QAction(QIcon::fromTheme(QLatin1String("go-next-view-page")), i18n("Insert Page Break"), collection);
    collection->addAction(QLatin1String("insert_page_break_entry"), insertPageBreakEntry);
    connect(insertPageBreakEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertPageBreakEntry()));

    QAction* insertImageEntry = new QAction(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("Insert Image"), collection);
    collection->addAction(QLatin1String("insert_image_entry"), insertImageEntry);
    connect(insertImageEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertImageEntry()));

    QAction* removeCurrent = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove current Entry"), collection);
    collection->addAction(QLatin1String("remove_current"), removeCurrent);
    collection->setDefaultShortcut(removeCurrent, Qt::ShiftModifier + Qt::Key_Delete);
    connect(removeCurrent, SIGNAL(triggered()), m_worksheet, SLOT(removeCurrentEntry()));

    m_showBackendHelp = new QAction(i18n("Show %1 Help", b->name()) , collection);
    m_showBackendHelp->setIcon(QIcon::fromTheme(QLatin1String("help-contents")));
    collection->addAction(QLatin1String("backend_help"), m_showBackendHelp);
    connect(m_showBackendHelp, SIGNAL(triggered()), this, SLOT(showBackendHelp()));

    QAction* publishWorksheet = new QAction(i18n("Publish Worksheet"), collection);
    publishWorksheet->setIcon(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")));
    collection->addAction(QLatin1String("file_publish_worksheet"), publishWorksheet);
    connect(publishWorksheet, SIGNAL(triggered()), this, SLOT(publishWorksheet()));

    KToggleAction* showEditor = new KToggleAction(i18n("Show Script Editor"), collection);
    showEditor->setChecked(false);
    collection->addAction(QLatin1String("show_editor"), showEditor);
    connect(showEditor, SIGNAL(toggled(bool)), this, SLOT(showScriptEditor(bool)));
    showEditor->setEnabled(b->extensions().contains(QLatin1String("ScriptExtension")));

    QAction* showCompletion = new QAction(i18n("Show Completion"), collection);
    collection->addAction(QLatin1String("show_completion"), showCompletion);
    QList<QKeySequence> showCompletionShortcuts;
    showCompletionShortcuts << Qt::Key_Tab << Qt::CTRL + Qt::Key_Space;
    collection->setDefaultShortcuts(showCompletion, showCompletionShortcuts);
    connect(showCompletion, SIGNAL(triggered()), m_worksheet, SLOT(showCompletion()));

    // set our XML-UI resource file
    setXMLFile(QLatin1String("cantor_part.rc"));

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

    initialized();
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

    // if so, we either enable or disable it based on the current state
    m_save->setEnabled(modified);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

KAboutData& CantorPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the Makefile)
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
    const bool rc = m_worksheet->load(localFilePath());
    QApplication::restoreOverrideCursor();

    if (rc) {
        qDebug()<< "Worksheet successfully loaded in " <<  (float)timer.elapsed()/1000 << " seconds).";
        updateCaption();
        // We modified, but it we load file now, so no need in save option
        setModified(false);
    }

    return rc;
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
        filter+=QLatin1String(";;")+e->scriptFileFilter();
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
    QString file_name = QFileDialog::getSaveFileName(widget(), i18n("Export to LaTeX"), QString(), QString());

    if (file_name.isEmpty() == false)
        {
        if (!file_name.endsWith(QLatin1String(".tex")))
            file_name += QLatin1String(".tex");
        m_worksheet->saveLatex(file_name);
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
    unsigned int count = ++m_sessionStatusCounter;
    if(status==Cantor::Session::Running)
    {
        // Useless add a interrupt action without delay, because user physically can't interrupt fast commands
        QTimer::singleShot(100, this, [this, count] () {
            if(m_worksheet->session()->status() == Cantor::Session::Running && m_sessionStatusCounter == count)
            {
                m_evaluate->setText(i18n("Interrupt"));
                m_evaluate->setShortcut(Qt::CTRL+Qt::Key_I);
                m_evaluate->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
                setStatusMessage(i18n("Calculating..."));
            }
        });
    }else if (status==Cantor::Session::Done)
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

void CantorPart::initialized()
{
    connect(m_worksheet->session(), SIGNAL(statusChanged(Cantor::Session::Status)), this, SLOT(worksheetStatusChanged(Cantor::Session::Status)));
    connect(m_worksheet->session(), SIGNAL(loginStarted()),this, SLOT(worksheetSessionLoginStarted()));
    connect(m_worksheet->session(), SIGNAL(loginDone()),this, SLOT(worksheetSessionLoginDone()));
    connect(m_worksheet->session(), SIGNAL(error(QString)), this, SLOT(showSessionError(QString)));

    loadAssistants();
    m_panelHandler->setSession(m_worksheet->session());
    adjustGuiToSession();

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

    m_typeset->setEnabled(true);

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

    emit setCaption(filename, QIcon::fromTheme(m_worksheet->session()->backend()->icon()));
}

void CantorPart::pluginsChanged()
{
    for (auto* plugin : m_panelHandler->plugins())
        connect(plugin, SIGNAL(requestRunCommand(QString)), this, SLOT(runCommand(QString)));
}

void CantorPart::loadAssistants()
{
    qDebug()<<"loading assistants...";

    QStringList assistantDirs;
    for (const QString& dir : QCoreApplication::libraryPaths())
        assistantDirs << dir + QDir::separator() + QLatin1String("cantor/assistants");

    QPluginLoader loader;
    for (const QString& dir : assistantDirs) {

        qDebug() << "dir: " << dir;
        QStringList assistants;
        QDir assistantDir = QDir(dir);

        assistants = assistantDir.entryList();

        for (const QString& assistant : assistants) {
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

            bool supported=true;
            for (const QString& req : plugin->requiredExtensions())
                supported=supported && backend->extensions().contains(req);

            if(supported)
            {
                qDebug() << "plugin " << info.name() << " is supported by " << backend->name() << ", requires extensions " << plugin->requiredExtensions();
                plugin->initActions();
                connect(plugin, SIGNAL(requested()), this, SLOT(runAssistant()));
            }else
            {
                qDebug() << "plugin " << info.name() << " is not supported by "<<backend->name();
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
    Cantor::Backend::Capabilities capabilities = m_worksheet->session()->backend()->capabilities();
#ifdef WITH_EPS
    m_typeset->setVisible(capabilities.testFlag(Cantor::Backend::LaTexOutput));
#else
    m_typeset->setVisible(false);
#endif
    m_completion->setVisible(capabilities.testFlag(Cantor::Backend::Completion));

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
        connect(m_scriptEditor, SIGNAL(runScript(QString)), this, SLOT(runScript(QString)));
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
