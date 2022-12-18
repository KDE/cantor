/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2017-2022 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2019 Sirgienko Nikita <warquark@gmail.com>
*/

#include <config-cantor.h>

#include <array>
#include <cmath>

#include "cantor_part.h"
#include "lib/assistant.h"
#include "lib/backend.h"
#include "lib/extension.h"
#include "lib/worksheetaccess.h"
#include "scripteditor/scripteditorwidget.h"
#include "searchbar.h"
#include "settings.h"
#include "worksheet.h"
#include "worksheetview.h"

#include <KAboutData>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNS3/UploadDialog>
#include <KParts/GUIActivateEvent>
#include <KPluginFactory>
#include <KIO/OpenUrlJob>
#include <KIO/JobUiDelegate>
#include <KStandardAction>
#include <KToggleAction>
#include <KSelectAction>
#include <KXMLGUIFactory>
#include <KZip>

#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QProgressDialog>
#include <QTextStream>
#include <QTimer>
#include <QRegularExpression>

//A concrete implementation of the WorksheetAccesssInterface
class WorksheetAccessInterfaceImpl : public Cantor::WorksheetAccessInterface
{

  public:
    WorksheetAccessInterfaceImpl(QObject* parent, Worksheet* worksheet) :   WorksheetAccessInterface(parent),  m_worksheet(worksheet)
    {
        qDebug()<<"new worksheetaccess interface";
        connect(worksheet, &Worksheet::modified, this, &WorksheetAccessInterfaceImpl::modified);
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

CantorPart::CantorPart(QWidget* parentWidget, QObject* parent, const QVariantList& args ): KParts::ReadWritePart(parent)
{
    QString backendName;
    if(!args.isEmpty())
        backendName = args.first().toString();

    Cantor::Backend* b = nullptr;
    if (!backendName.isEmpty())
    {
        b = Cantor::Backend::getBackend(backendName);
        if (b)
            qDebug()<<"Backend "<<b->name()<<" offers extensions: "<<b->extensions();
    }

    //central widget
    QWidget* widget = new QWidget(parentWidget);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    m_worksheet = new Worksheet(b, widget);
    m_worksheetview = new WorksheetView(m_worksheet, widget);
    m_worksheetview->setEnabled(false); //disable input until the session has successfully logged in and emits the ready signal
    connect(m_worksheet, &Worksheet::modified, this, static_cast<void (KParts::ReadWritePart::*)()>(&KParts::ReadWritePart::setModified));
    connect(m_worksheet, &Worksheet::modified, this, &CantorPart::updateCaption);
    connect(m_worksheet, &Worksheet::showHelp, this, &CantorPart::showHelp);
    connect(m_worksheet, &Worksheet::loaded, this, &CantorPart::initialized);
    connect(m_worksheet, &Worksheet::hierarchyChanged, this, &CantorPart::hierarchyChanged);
    connect(m_worksheet, &Worksheet::hierarhyEntryNameChange, this, &CantorPart::hierarhyEntryNameChange);
    connect(this, &CantorPart::requestScrollToHierarchyEntry, m_worksheet, &Worksheet::requestScrollToHierarchyEntry);
    connect(this, &CantorPart::settingsChanges, m_worksheet, &Worksheet::handleSettingsChanges);
    connect(m_worksheet, &Worksheet::requestDocumentation, this, &CantorPart::documentationRequested);

    layout->addWidget(m_worksheetview);
    setWidget(widget);

    //create WorksheetAccessInterface, used at the moment by LabPlot only to access Worksheet's API
    Cantor::WorksheetAccessInterface* iface = new WorksheetAccessInterfaceImpl(this, m_worksheet);
    Q_UNUSED(iface);

    //initialize actions
    auto* collection = actionCollection();
    connect(collection, &KActionCollection::inserted, m_worksheet, &Worksheet::registerShortcut);
    m_worksheet->setActionCollection(collection);

    KStandardAction::saveAs(this, SLOT(fileSaveAs()), collection);
    m_save = KStandardAction::save(this, SLOT(save()), collection);
    m_save->setPriority(QAction::LowPriority);

    QAction* savePlain = new QAction(i18n("Save Plain Text"), collection);
    collection->addAction(QLatin1String("file_save_plain"), savePlain);
    savePlain->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    connect(savePlain, &QAction::triggered, this, &CantorPart::fileSavePlain);

    QAction* undo = KStandardAction::undo(m_worksheet, SIGNAL(undo()), collection);
    undo->setPriority(QAction::LowPriority);
    connect(m_worksheet, &Worksheet::undoAvailable, undo, &QAction::setEnabled);
    m_editActions.push_back(undo);

    QAction* redo = KStandardAction::redo(m_worksheet, SIGNAL(redo()), collection);
    redo->setPriority(QAction::LowPriority);
    connect(m_worksheet, &Worksheet::redoAvailable, redo, &QAction::setEnabled);
    m_editActions.push_back(redo);

    QAction* cut = KStandardAction::cut(m_worksheet, SIGNAL(cut()), collection);
    cut->setPriority(QAction::LowPriority);
    connect(m_worksheet, &Worksheet::cutAvailable, cut, &QAction::setEnabled);
    m_editActions.push_back(cut);

    QAction* copy = KStandardAction::copy(m_worksheet, SIGNAL(copy()), collection);
    copy->setPriority(QAction::LowPriority);
    connect(m_worksheet, &Worksheet::copyAvailable, copy, &QAction::setEnabled);

    QAction* paste = KStandardAction::paste(m_worksheet, SLOT(paste()), collection);
    paste->setPriority(QAction::LowPriority);
    connect(m_worksheet, &Worksheet::pasteAvailable, paste, &QAction::setEnabled);
    m_editActions.push_back(paste);

    QAction* find = KStandardAction::find(this, SLOT(showSearchBar()), collection);
    find->setPriority(QAction::LowPriority);

    QAction* replace = KStandardAction::replace(this, SLOT(showExtendedSearchBar()), collection);
    replace->setPriority(QAction::LowPriority);
    m_editActions.push_back(replace);

    m_findNext = KStandardAction::findNext(this, SLOT(findNext()), collection);
    m_findNext->setEnabled(false);

    m_findPrev = KStandardAction::findPrev(this, SLOT(findPrev()), collection);
    m_findPrev->setEnabled(false);

    QAction* latexExport = new QAction(i18n("Export to LaTeX"), collection);
    collection->addAction(QLatin1String("file_export_latex"), latexExport);
    latexExport->setIcon(QIcon::fromTheme(QLatin1String("document-export")));
    connect(latexExport, &QAction::triggered, this, &CantorPart::exportToLatex);

    QAction* print = KStandardAction::print(this, SLOT(print()), collection);
    print->setPriority(QAction::LowPriority);

    QAction* printPreview = KStandardAction::printPreview(this, SLOT(printPreview()), collection);
    printPreview->setPriority(QAction::LowPriority);

    KStandardAction::zoomIn(m_worksheetview, SLOT(zoomIn()), collection);
    KStandardAction::zoomOut(m_worksheetview, SLOT(zoomOut()), collection);
    KStandardAction::actualSize(m_worksheetview, SLOT(actualSize()), collection);
    connect(m_worksheetview, &WorksheetView::scaleFactorChanged, this, &CantorPart::updateZoomWidgetValue);

    m_evaluate = new QAction(i18n("Evaluate Worksheet"), collection);
    collection->addAction(QLatin1String("evaluate_worksheet"), m_evaluate);
    m_evaluate->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    collection->setDefaultShortcut(m_evaluate, Qt::CTRL+Qt::Key_E);
    connect(m_evaluate, &QAction::triggered, this, &CantorPart::evaluateOrInterrupt);
    m_editActions.push_back(m_evaluate);

    //
    m_zoom = new KSelectAction(QIcon::fromTheme(QLatin1String("page-zoom")), i18n("Zoom"), collection);
    connect(m_zoom, static_cast<void (KSelectAction::*)(const QString&)>(&KSelectAction::textTriggered), this, &CantorPart::zoomValueEdited);
    static constexpr std::array<double, 8> ZoomValues = {0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 4.0};
    QStringList zoomNames;
    for (double zoomValue : ZoomValues)
    {
        const std::string& zoomName = std::to_string(static_cast<int>(zoomValue * 100)) + "%";
        zoomNames << i18n(zoomName.c_str());
    }
    m_zoom->setItems(zoomNames);
    m_zoom->setEditable(true);
    Q_ASSERT(std::find(ZoomValues.begin(), ZoomValues.end(), 1.0) != ZoomValues.end());
    m_zoom->setCurrentItem(std::distance(ZoomValues.begin(), std::find(ZoomValues.begin(), ZoomValues.end(), 1.0)));
    collection->addAction(QLatin1String("zoom_selection_action"), m_zoom);

    m_typeset = new KToggleAction(i18n("Typeset using LaTeX"), collection);
    m_typeset->setChecked(Settings::self()->typesetDefault());
    collection->addAction(QLatin1String("enable_typesetting"), m_typeset);
    connect(m_typeset, &KToggleAction::toggled, this, &CantorPart::enableTypesetting);

    m_highlight = new KToggleAction(i18n("Syntax Highlighting"), collection);
    m_highlight->setChecked(Settings::self()->highlightDefault());
    collection->addAction(QLatin1String("enable_highlighting"), m_highlight);
    connect(m_highlight, &KToggleAction::toggled, m_worksheet, &Worksheet::enableHighlighting);

    m_completion = new KToggleAction(i18n("Completion"), collection);
    m_completion->setChecked(Settings::self()->completionDefault());
    collection->addAction(QLatin1String("enable_completion"), m_completion);
    connect(m_completion, &KToggleAction::toggled, m_worksheet, &Worksheet::enableCompletion);

    m_exprNumbering = new KToggleAction(i18n("Line Numbers"), collection);
    m_exprNumbering->setChecked(Settings::self()->expressionNumberingDefault());
    collection->addAction(QLatin1String("enable_expression_numbers"), m_exprNumbering);
    connect(m_exprNumbering, &KToggleAction::toggled, m_worksheet, &Worksheet::enableExpressionNumbering);

    m_animateWorksheet = new KToggleAction(i18n("Animations"), collection);
    m_animateWorksheet->setChecked(Settings::self()->animationDefault());
    collection->addAction(QLatin1String("enable_animations"), m_animateWorksheet);
    connect(m_animateWorksheet, &KToggleAction::toggled, m_worksheet, &Worksheet::enableAnimations);

    if (MathRenderer::mathRenderAvailable())
    {
        m_embeddedMath= new KToggleAction(i18n("Embedded Math"), collection);
        m_embeddedMath->setChecked(Settings::self()->embeddedMathDefault());
        collection->addAction(QLatin1String("enable_embedded_math"), m_embeddedMath);
        connect(m_embeddedMath, &KToggleAction::toggled, m_worksheet, &Worksheet::enableEmbeddedMath);
    }

    m_restart = new QAction(i18n("Restart Backend"), collection);
    collection->addAction(QLatin1String("restart_backend"), m_restart);
    m_restart->setIcon(QIcon::fromTheme(QLatin1String("system-reboot")));
    connect(m_restart, &QAction::triggered, this, &CantorPart::restartBackend);
    m_restart->setEnabled(false); // No need show restart button before login
    m_editActions.push_back(m_restart);

    QAction* evaluateCurrent = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Evaluate Entry"), collection);
    collection->addAction(QLatin1String("evaluate_current"),  evaluateCurrent);
    collection->setDefaultShortcut(evaluateCurrent, Qt::SHIFT + Qt::Key_Return);
    connect(evaluateCurrent, &QAction::triggered, m_worksheet, &Worksheet::evaluateCurrentEntry);
    m_editActions.push_back(evaluateCurrent);

    QAction* insertCommandEntry = new QAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Insert Command Entry"), collection);
    collection->addAction(QLatin1String("insert_command_entry"),  insertCommandEntry);
    collection->setDefaultShortcut(insertCommandEntry, Qt::CTRL + Qt::Key_Return);
    connect(insertCommandEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertCommandEntry()));
    m_editActions.push_back(insertCommandEntry);

    QAction* insertTextEntry = new QAction(QIcon::fromTheme(QLatin1String("draw-text")), i18n("Insert Text Entry"), collection);
    collection->addAction(QLatin1String("insert_text_entry"),  insertTextEntry);
    connect(insertTextEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertTextEntry()));
    m_editActions.push_back(insertTextEntry);

#ifdef Discount_FOUND
    QAction* insertMarkdownEntry = new QAction(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Insert Markdown Entry"), collection);
    collection->addAction(QLatin1String("insert_markdown_entry"),  insertMarkdownEntry);
    connect(insertMarkdownEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertMarkdownEntry()));
    m_editActions.push_back(insertMarkdownEntry);
#endif

#ifdef WITH_EPS
    QAction* insertLatexEntry = new QAction(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("Insert LaTeX Entry"), collection);
    collection->addAction(QLatin1String("insert_latex_entry"),  insertLatexEntry);
    connect(insertLatexEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertLatexEntry()));
    m_editActions.push_back(insertLatexEntry);
#endif

    QAction* insertPageBreakEntry = new QAction(QIcon::fromTheme(QLatin1String("go-next-view-page")), i18n("Insert Page Break"), collection);
    collection->addAction(QLatin1String("insert_page_break_entry"), insertPageBreakEntry);
    connect(insertPageBreakEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertPageBreakEntry()));
    m_editActions.push_back(insertPageBreakEntry);

    QAction* insertImageEntry = new QAction(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("Insert Image"), collection);
    collection->addAction(QLatin1String("insert_image_entry"), insertImageEntry);
    connect(insertImageEntry, SIGNAL(triggered()), m_worksheet, SLOT(insertImageEntry()));
    m_editActions.push_back(insertImageEntry);

    QAction* collapseAllEntries = new QAction(QIcon(), i18n("Collapse All Results"), collection);
    collection->addAction(QLatin1String("all_entries_collapse_results"), collapseAllEntries);
    connect(collapseAllEntries, &QAction::triggered, m_worksheet, &Worksheet::collapseAllResults);
    m_editActions.push_back(collapseAllEntries);

    QAction* uncollapseAllEntries = new QAction(QIcon(), i18n("Expand All Results"), collection);
    collection->addAction(QLatin1String("all_entries_uncollapse_results"), uncollapseAllEntries );
    connect(uncollapseAllEntries , &QAction::triggered, m_worksheet, &Worksheet::uncollapseAllResults);
    m_editActions.push_back(uncollapseAllEntries);

    QAction* removeAllResults = new QAction(QIcon(), i18n("Remove All Results"), collection);
    collection->addAction(QLatin1String("all_entries_remove_all_results"), removeAllResults);
    connect(removeAllResults, &QAction::triggered, m_worksheet, &Worksheet::removeAllResults);
    m_editActions.push_back(removeAllResults);

    QAction* removeCurrent = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove current Entry"), collection);
    collection->addAction(QLatin1String("remove_current"), removeCurrent);
    collection->setDefaultShortcut(removeCurrent, Qt::ShiftModifier + Qt::Key_Delete);
    connect(removeCurrent, &QAction::triggered, m_worksheet, &Worksheet::removeCurrentEntry);
    m_editActions.push_back(removeCurrent);

    // Disabled, because uploading to kde store from program don't work
    // See https://phabricator.kde.org/T9980 for details
    // If this situation will changed, then uncomment this action
    /*
    QAction* publishWorksheet = new QAction(i18n("Publish Worksheet"), collection);
    publishWorksheet->setIcon(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")));
    collection->addAction(QLatin1String("file_publish_worksheet"), publishWorksheet);
    connect(publishWorksheet, &QAction::triggered, this, SLOT(publishWorksheet()));
    */

    KToggleAction* showEditor = new KToggleAction(i18n("Show Script Editor"), collection);
    showEditor->setChecked(false);
    collection->addAction(QLatin1String("show_editor"), showEditor);
    connect(showEditor, &KToggleAction::toggled, this, &CantorPart::showScriptEditor);

    QAction* showCompletion = new QAction(i18n("Show Completion"), collection);
    collection->addAction(QLatin1String("show_completion"), showCompletion);
    collection->setDefaultShortcut(showCompletion, Qt::CTRL + Qt::Key_Space);
    connect(showCompletion, &QAction::triggered, m_worksheet, &Worksheet::showCompletion);
    m_editActions.push_back(showCompletion);

    // set our XML-UI resource file
    setXMLFile(QLatin1String("cantor_part.rc"));

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

    if (b)
    {
        showEditor->setEnabled(b->extensions().contains(QLatin1String("ScriptExtension")));
        initialized();
    }
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

void CantorPart::setReadOnly()
{
    for (QAction* action : m_editActions)
        action->setEnabled(false);
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
                     QLatin1String("https://edu.kde.org/cantor/"));

    about.addAuthor( i18n("Alexander Rieder"), QString(), QLatin1String("alexanderrieder@gmail.com") );
    return about;
}

bool CantorPart::openFile()
{
    //don't crash if for some reason the worksheet is invalid
    if(!m_worksheet)
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
        qDebug()<< "Worksheet successfully loaded in " <<  (float)timer.elapsed()/1000 << " seconds";
        updateCaption();
        if (m_worksheet->session() && m_worksheet->session()->backend())
            setBackendName(m_worksheet->session()->backend()->id());
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
    updateCaption();

    emit worksheetSave(QUrl::fromLocalFile(localFilePath()));
    return true;
}

void CantorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected
    static const QString& worksheetFilter = i18n("Cantor Worksheet (*.cws)");
    static const QString& notebookFilter = i18n("Jupyter Notebook (*.ipynb)");
    QString filter = worksheetFilter + QLatin1String(";;") + notebookFilter;

    if (!m_worksheet->isReadOnly())
    {
        //if the backend supports scripts, also append their scriptFile endings to the filter
        auto* const backend = m_worksheet->session()->backend();
        if (backend->extensions().contains(QLatin1String("ScriptExtension")))
        {
            auto* e = dynamic_cast<Cantor::ScriptExtension*>(backend->extension(QLatin1String("ScriptExtension")));
            if (e)
                filter += QLatin1String(";;") + e->scriptFileFilter();
        }
    }

    QString selectedFilter;
    QString file_name = QFileDialog::getSaveFileName(widget(), i18n("Save as"), QString(), filter, &selectedFilter);
    if (file_name.isEmpty())
        return;

    static const QString jupyterExtension = QLatin1String(".ipynb");
    static const QString cantorExtension = QLatin1String(".cws");
    // Append file extension, if it isn't specified
    // And change filter, if it specified to supported extension
    if (file_name.contains(QLatin1String(".")))
    {
        if (file_name.endsWith(cantorExtension))
            selectedFilter = worksheetFilter;
        else if (file_name.endsWith(jupyterExtension))
            selectedFilter = notebookFilter;
    }
    else
    {
        if (selectedFilter == worksheetFilter)
            file_name += cantorExtension;
        else if (selectedFilter == notebookFilter)
            file_name += jupyterExtension;
    }

    //depending on user's selection, save as a worksheet, as a Jupyter notebook or as a plain script file
    if (selectedFilter == worksheetFilter)
    {
        m_worksheet->setType(Worksheet::CantorWorksheet);
        const QUrl& url = QUrl::fromLocalFile(file_name);
        saveAs(url);
        emit worksheetSave(url);
    }
    else if (selectedFilter == notebookFilter)
    {
        m_worksheet->setType(Worksheet::JupyterNotebook);
        const QUrl& url = QUrl::fromLocalFile(file_name);
        saveAs(url);
        emit worksheetSave(url);
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
    bool restart = false;
    if (Settings::self()->warnAboutSessionRestart())
    {
        KMessageBox::ButtonCode tmp;

        // If we want the question box, but it is disable, then enable it
        if (!KMessageBox::shouldBeShownYesNo(QLatin1String("WarnAboutSessionRestart"), tmp))
            KMessageBox::enableMessage(QLatin1String("WarnAboutSessionRestart"));

        const QString& name = m_worksheet->session()->backend()->name();
        KMessageBox::ButtonCode rc = KMessageBox::questionYesNo(widget(),
            i18n("All the available calculation results will be lost. Do you really want to restart %1?", name),
            i18n("Restart %1?", name),
            KStandardGuiItem::yes(),
            KStandardGuiItem::no(),
            QLatin1String("WarnAboutSessionRestart")
        );

        // Update setting's value
        // I don't know, that should I do with "No" with "Don't ask me again"
        // So hide warning only on "Yes"
        Settings::self()->setWarnAboutSessionRestart(
               KMessageBox::shouldBeShownYesNo(QLatin1String("WarnAboutSessionRestart"), tmp)
            || rc == KMessageBox::ButtonCode::No
        );
        Settings::self()->save();

        restart = (rc == KMessageBox::ButtonCode::Yes);
    }
    else
    {
        restart = true;
    }

    if (restart)
    {
        m_worksheet->session()->logout();
        m_worksheet->loginToSession();
    }
}

void CantorPart::worksheetStatusChanged(Cantor::Session::Status status)
{
    qDebug()<<"worksheet status changed:" << status;
    unsigned int count = ++m_sessionStatusCounter;
    switch (status) {
    case Cantor::Session::Running:
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
        break;
    }
    case Cantor::Session::Done:
    {
        m_evaluate->setText(i18n("Evaluate Worksheet"));
        m_evaluate->setShortcut(Qt::CTRL+Qt::Key_E);
        m_evaluate->setIcon(QIcon::fromTheme(QLatin1String("system-run")));

        setStatusMessage(i18n("Ready"));
        break;
    }
    case Cantor::Session::Disable:
        setStatusMessage(QString()); //clean the status bar to remove the potential "Calculating...", etc. after the session was closed
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
    if (!m_worksheet->isReadOnly())
    {
        connect(m_worksheet->session(), &Cantor::Session::statusChanged, this, &CantorPart::worksheetStatusChanged);
        connect(m_worksheet->session(), &Cantor::Session::loginStarted,this, &CantorPart::worksheetSessionLoginStarted);
        connect(m_worksheet->session(), &Cantor::Session::loginDone,this, &CantorPart::worksheetSessionLoginDone);
        connect(m_worksheet->session(), &Cantor::Session::error, this, &CantorPart::showSessionError);

        loadAssistants();
        adjustGuiToSession();

        // Don't set modification flag, if we add command entry in empty worksheet
        const bool modified = this->isModified();
        if (m_worksheet->isEmpty())
            m_worksheet->appendCommandEntry();
        setModified(modified);
    }
    else
    {
        setReadOnly();
        // Clear assistants
        for (KXMLGUIClient* client: childClients())
        {
            Cantor::Assistant* assistant = dynamic_cast<Cantor::Assistant*>(client);
            if (assistant)
            {
                if (factory())
                    factory()->removeClient(client);
                removeChildClient(client);
                assistant->deleteLater();
            }
        }
    }

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
    m_restart->setEnabled(true);
    QApplication::restoreOverrideCursor();
}

void CantorPart::enableTypesetting(bool enable)
{
    m_worksheet->session()->setTypesettingEnabled(enable);
}

/*!
 * called when the current worksheet has requested to show the documentation for \c keyword.
 * In case the local documentation is available for the current backend, the signal is
 * forwarded to the shell to show the documentation plugin/widget.
 * If no local documentation is available, the defaul online URL for the backend documentation
 * is openned.
 */
void CantorPart::documentationRequested(const QString& keyword) {
    auto* backend = m_worksheet->session()->backend();
    const auto& group = KSharedConfig::openConfig(QStringLiteral("cantorrc"))->group(backend->name().toLower());
    const auto& docNames = group.readEntry(QLatin1String("Names"), QStringList());
    if (!docNames.isEmpty())
        emit requestDocumentation(keyword);
    else
        showBackendHelp();
}

void CantorPart::showBackendHelp()
{
    auto* backend = m_worksheet->session()->backend();
    auto* job = new KIO::OpenUrlJob(backend->helpUrl());
    job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, widget()));
    job->start();
    delete job;
}

Worksheet* CantorPart::worksheet()
{
    return m_worksheet;
}

void CantorPart::updateCaption()
{
    QString filename = url().fileName();
    //strip away the extension
    filename=filename.left(filename.lastIndexOf(QLatin1Char('.')));

    if (!m_worksheet->isReadOnly())
    {
        if (m_worksheet->session())
            emit setCaption(filename, QIcon::fromTheme(m_worksheet->session()->backend()->icon()));
    }
    else
        emit setCaption(filename+QLatin1Char(' ') + i18n("[read-only]"), QIcon());
}

void CantorPart::loadAssistants()
{
    qDebug()<<"loading assistants...";

    const QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("cantor/assistants"));

    for (const KPluginMetaData &plugin : plugins) {

        const auto result = KPluginFactory::instantiatePlugin<Cantor::Assistant>(plugin, this);

        if (!result) {
            qDebug() << "Error while loading assistant plugin: " << result.errorText;
            continue;
        }

        Cantor::Assistant *assistant = result.plugin;
        auto* backend=worksheet()->session()->backend();
        assistant->setPluginInfo(plugin);
        assistant->setBackend(backend);

        bool supported=true;
        for (const QString& req : assistant->requiredExtensions())
            supported = supported && backend->extensions().contains(req);

        if(supported)
        {
            qDebug() << "plugin " << plugin.name() << " is supported by " << backend->name() << ", requires extensions " << assistant->requiredExtensions();
            assistant->initActions();
            connect(assistant, &Cantor::Assistant::requested, this, &CantorPart::runAssistant);
        }else
        {
            qDebug() << "plugin " << plugin.name() << " is not supported by "<<backend->name();
            removeChildClient(assistant);
            assistant->deleteLater();
        }
    }
}

void CantorPart::runAssistant()
{
    Cantor::Assistant* a = qobject_cast<Cantor::Assistant*>(sender());
    QStringList cmds = a->run(widget());
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
        connect(m_searchBar, &SearchBar::destroyed, this, &CantorPart::searchBarDeleted);
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
        connect(m_searchBar, &SearchBar::destroyed, this, &CantorPart::searchBarDeleted);
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
    auto capabilities = m_worksheet->session()->backend()->capabilities();
#ifdef WITH_EPS
    if (Cantor::LatexRenderer::isLatexAvailable())
        m_typeset->setVisible(capabilities.testFlag(Cantor::Backend::LaTexOutput));
#else
    m_typeset->setVisible(false);
#endif
    m_completion->setVisible(capabilities.testFlag(Cantor::Backend::Completion));
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
    Q_UNUSED(dialog.exec());
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
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(widget());
    connect(dialog, &QPrintPreviewDialog::paintRequested, m_worksheet, &Worksheet::print);
    Q_UNUSED(dialog->exec());
}

void CantorPart::showScriptEditor(bool show)
{
    if(show)
    {
        if (m_scriptEditor)
            return;

        auto* scriptE = dynamic_cast<Cantor::ScriptExtension*>(m_worksheet->session()->backend()->extension(QLatin1String("ScriptExtension")));
        if (!scriptE)
            return;

        m_scriptEditor = new ScriptEditorWidget(scriptE->scriptFileFilter(), scriptE->highlightingMode(), widget()->window());
        connect(m_scriptEditor, &ScriptEditorWidget::runScript, this, &CantorPart::runScript);
        connect(m_scriptEditor, &ScriptEditorWidget::destroyed, this, &CantorPart::scriptEditorClosed);
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
    auto* backend = m_worksheet->session()->backend();
    if(!backend->extensions().contains(QLatin1String("ScriptExtension")))
    {
        KMessageBox::error(widget(), i18n("This backend does not support scripts."), i18n("Error - Cantor"));
        return;
    }

    auto* scriptE = dynamic_cast<Cantor::ScriptExtension*>(backend->extension(QLatin1String("ScriptExtension")));
    if (scriptE)
        m_worksheet->appendCommandEntry(scriptE->runExternalScript(file));
}

void CantorPart::blockStatusBar()
{
    m_statusBarBlocked=true;
}

void CantorPart::unblockStatusBar()
{
    m_statusBarBlocked = false;
    if(!m_cachedStatusMessage.isNull())
        setStatusMessage(m_cachedStatusMessage);
    m_cachedStatusMessage.clear();
}

void CantorPart::setStatusMessage(const QString& message)
{
    if(!m_statusBarBlocked)
        emit setStatusBarText(message);
    else
        m_cachedStatusMessage = message;
}

void CantorPart::showImportantStatusMessage(const QString& message)
{
    setStatusMessage(message);
    blockStatusBar();
    QTimer::singleShot(3000, this, SLOT(unblockStatusBar()));
}

void CantorPart::zoomValueEdited(const QString& text)
{
    static const QRegularExpression zoomRegexp(QLatin1String("(?:(\\d+)%|(\\d+))"));
    QRegularExpressionMatch match = zoomRegexp.match(text);
    if (match.hasMatch())
    {
        double zoom = match.captured(1).toDouble() / 100.0;
        if (m_worksheetview)
            m_worksheetview->setScaleFactor(zoom, false);
    }
}

void CantorPart::updateZoomWidgetValue(double zoom)
{
    if (m_zoom)
    {
        double scale = zoom;
        scale = round(scale * 100);
        const QString& searchText = QString::number((int)scale) + QLatin1String("%");
        if (m_currectZoomAction)
            m_currectZoomAction->setText(searchText);
        else
            m_currectZoomAction = m_zoom->addAction(searchText);
        m_zoom->setCurrentAction(m_currectZoomAction);
    }
}

K_PLUGIN_FACTORY_WITH_JSON(CantorPartFactory, "cantor_part.json", registerPlugin<CantorPart>();)
#include "cantor_part.moc"
