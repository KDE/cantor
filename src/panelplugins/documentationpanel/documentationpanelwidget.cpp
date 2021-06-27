/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020-2021 Alexander Semke <alexander.semke@web.de>
*/

#include "cantor_macros.h"
#include "documentationpanelwidget.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QAction>
#include <QCompleter>
#include <QComboBox>
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QPushButton>
#include <QShortcut>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWebEngineDownloadItem>
#include <QWebEngineProfile>
#include <QWebEngineUrlScheme>
#include <QWebEngineView>

DocumentationPanelWidget::DocumentationPanelWidget(QWidget* parent) : QWidget(parent)
{
    m_webEngineView = new QWebEngineView(this);
    m_webEngineView->page()->action(QWebEnginePage::ViewSource)->setVisible(false);
    m_webEngineView->page()->action(QWebEnginePage::OpenLinkInNewTab)->setVisible(false);
    m_webEngineView->page()->action(QWebEnginePage::OpenLinkInNewWindow)->setVisible(false);
    m_webEngineView->page()->action(QWebEnginePage::DownloadLinkToDisk)->setVisible(false);
    m_webEngineView->page()->action(QWebEnginePage::Reload)->setVisible(false);

    /////////////////////////
    // Top toolbar layout //
    ///////////////////////
    QPushButton* home = new QPushButton(this);
    home->setIcon(QIcon::fromTheme(QLatin1String("go-home")));
    home->setToolTip(i18nc("@button go to contents page", "Go to the contents"));
    home->setEnabled(false);

    m_documentationSelector = new QComboBox(this);

    // real time searcher
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(i18nc("@info:placeholder", "Search through keywords..."));
    m_search->setClearButtonEnabled(true);

    // Add a seperator
    QFrame* seperator = new QFrame(this);
    seperator->setFrameShape(QFrame::VLine);
    seperator->setFrameShadow(QFrame::Sunken);

    QPushButton* findPage = new QPushButton(this);
    findPage->setEnabled(false);
    findPage->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    findPage->setToolTip(i18nc("@info:tooltip", "Find in text of current documentation page"));
    findPage->setShortcut(QKeySequence(/*Qt::CTRL + */Qt::Key_F3));

    QPushButton* resetZoom = new QPushButton(this);
    resetZoom->setEnabled(false);
    resetZoom->setIcon(QIcon::fromTheme(QLatin1String("zoom-fit-best")));
    resetZoom->setToolTip(i18nc("@info:tooltip", "Reset zoom level to 100%"));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(home);
    layout->addWidget(m_documentationSelector);
    layout->addWidget(m_search);
    layout->addWidget(seperator);
    layout->addWidget(findPage);
    layout->addWidget(resetZoom);

    QWidget* toolBarContainer = new QWidget(this);
    toolBarContainer->setLayout(layout);

    // Add zoom in, zoom out behaviour on SHIFT++ and SHIFT--
    auto zoomIn = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Plus), this);
    zoomIn->setContext(Qt::WidgetWithChildrenShortcut);

    connect(zoomIn, &QShortcut::activated, this, [=]{
        m_webEngineView->setZoomFactor(m_webEngineView->zoomFactor() + 0.1);
        emit zoomFactorChanged();
    });

    auto zoomOut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Minus), this);
    zoomOut->setContext(Qt::WidgetWithChildrenShortcut);

    connect(zoomOut, &QShortcut::activated, this, [=]{
        m_webEngineView->setZoomFactor(m_webEngineView->zoomFactor() - 0.1);
        emit zoomFactorChanged();
    });

    connect(this, &DocumentationPanelWidget::zoomFactorChanged, [=]{
        if(m_webEngineView->zoomFactor() != 1.0)
            resetZoom->setEnabled(true);
        else
            resetZoom->setEnabled(false);
    });

    //stack widget containing the web view and the content widget (will be added later in updateBacked())
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->addWidget(m_webEngineView);

    /////////////////////////////////
    // Find in Page widget layout //
    ///////////////////////////////

    // Add the Find in Page widget at the bottom, add all the widgets into a layout so that we can hide it
    QToolButton* hideButton = new QToolButton(this);
    hideButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
    hideButton->setToolTip(i18nc("@info:tooltip", "Close"));

    QLabel* label = new QLabel(this);
    label->setText(i18n("Find:"));

    m_findText = new QLineEdit(this);
    m_findText->setClearButtonEnabled(true);

    QToolButton* next = new QToolButton(this);
    next->setIcon(QIcon::fromTheme(QLatin1String("go-down-search")));
    next->setToolTip(i18nc("@info:tooltip", "Jump to next match"));

    QToolButton* previous = new QToolButton(this);
    previous->setIcon(QIcon::fromTheme(QLatin1String("go-up-search")));
    previous->setToolTip(i18nc("@info:tooltip", "Jump to previous match"));

    m_matchCase = new QToolButton(this);
    m_matchCase->setIcon(QIcon::fromTheme(QLatin1String("format-text-superscript")));
    m_matchCase->setToolTip(i18nc("@info:tooltip", "Match case sensitively"));
    m_matchCase->setCheckable(true);

    // Create a layout for find in text widgets
    QHBoxLayout* lout = new QHBoxLayout(this);
    lout->addWidget(hideButton);
    lout->addWidget(label);
    lout->addWidget(m_findText);
    lout->addWidget(next);
    lout->addWidget(previous);
    lout->addWidget(m_matchCase);

    QWidget* findPageWidgetContainer = new QWidget(this);
    findPageWidgetContainer->setLayout(lout);
    findPageWidgetContainer->hide();

    // Add topmost toolbar, display area and find in page widget in a Vertical layout
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->addWidget(toolBarContainer);
    vlayout->addWidget(m_stackedWidget);
    vlayout->addWidget(findPageWidgetContainer);

    connect(m_documentationSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] {
        updateDocumentation();
        m_stackedWidget->setCurrentIndex(1);
    });

    connect(m_stackedWidget, &QStackedWidget::currentChanged, [=]{
        //disable Home and Search in Page buttons when stackwidget shows contents widget, enable when shows web browser
        bool enabled = (m_stackedWidget->currentIndex() == 0); //0 = web view, 1 = content widget
        findPage->setEnabled(enabled);
        home->setEnabled(enabled);
    });

    connect(home, &QPushButton::clicked, [=]{
        m_stackedWidget->setCurrentIndex(1); //navigate to the content widget
        findPageWidgetContainer->hide();
    });

    connect(resetZoom, &QPushButton::clicked, [=]{
        m_webEngineView->setZoomFactor(1.0);
        resetZoom->setEnabled(false);
    });

    connect(m_search, &QLineEdit::returnPressed, this, &DocumentationPanelWidget::returnPressed);

    // connect statements for Find in Page text widget
    connect(findPage, &QPushButton::clicked, [=]{
        findPageWidgetContainer->show();
        m_findText->clear();
        m_findText->setFocus();
    });

    connect(hideButton, &QToolButton::clicked, this, [=]{
        findPageWidgetContainer->hide();
        m_webEngineView->findText(QString()); // this clears up the selected text
    });

    connect(m_findText, &QLineEdit::returnPressed, this, &DocumentationPanelWidget::searchForward);
    connect(m_findText, &QLineEdit::textEdited, this, &DocumentationPanelWidget::searchForward); // for highlighting found string in real time
    connect(next, &QToolButton::clicked, this, &DocumentationPanelWidget::searchForward);
    connect(previous, &QToolButton::clicked, this, &DocumentationPanelWidget::searchBackward);
    connect(m_matchCase, &QAbstractButton::toggled, this, &DocumentationPanelWidget::searchForward);
    connect(m_matchCase, &QAbstractButton::toggled, this, [=]{
        m_webEngineView->findText(QString());
        searchForward();
    });

    // for webenginebrowser for downloading of images or html pages
    connect(m_webEngineView->page()->profile(), &QWebEngineProfile::downloadRequested,
            this, &DocumentationPanelWidget::downloadResource);
}

DocumentationPanelWidget::~DocumentationPanelWidget()
{
    delete m_indexWidget;
    delete m_contentWidget;
    delete m_engine;
    delete m_webEngineView;
    delete m_stackedWidget;
    delete m_search;
    delete m_findText;
    delete m_matchCase;
    delete m_documentationSelector;
}

void DocumentationPanelWidget::updateBackend(const QString& newBackend)
{
    qDebug()<<"update backend " << newBackend;
    //nothing to do if the same backend was provided
    if(m_backend == newBackend)
        return;

    m_backend = newBackend;
    m_initializing = true;

    // show all available documentation files for the new backend
    m_documentationSelector->clear();
    const KConfigGroup& group = KSharedConfig::openConfig()->group(m_backend.toLower());
    m_docNames = group.readEntry(QLatin1String("Names"), QStringList());
    m_docPaths = group.readEntry(QLatin1String("Paths"), QStringList());
    const QStringList& iconNames = group.readEntry(QLatin1String("Icons"), QStringList());
    for (int i = 0; i < m_docNames.size(); ++i) {
        const QString& name = m_docNames.at(i);
        QString iconName;
        if (i < iconNames.size())
            iconName = iconNames.at(i);

        m_documentationSelector->addItem(QIcon::fromTheme(iconName), name);
    }

    m_initializing = false;

    //select the first available documentation file which will trigger the re-initialization of QHelpEngine
    //TODO: restore from the saved state the previously selected documentation in m_documentationSelector for the current backend
    if (!m_docNames.isEmpty())
        m_documentationSelector->setCurrentIndex(0);

    updateDocumentation();

    if (!m_docNames.isEmpty())
    {
        m_webEngineView->show();
        m_stackedWidget->setCurrentIndex(1);
    }
    else
        m_webEngineView->hide();
}

/*!
 * called if another documentation file was selected in the ComboBox for all available documentations
 * for the current backend. This slot triggers the re-initialization of QHelpEngine with the proper
 * documentation file and also updates the content of the widgets showing the documentation.
 */
void DocumentationPanelWidget::updateDocumentation()
{
    if (m_initializing)
        return;

    //remove the currently shown content widget, will be replaced with the new one after
    //the help engine was initialized with the new documentation file
    if(m_contentWidget)
    {
        m_stackedWidget->removeWidget(m_contentWidget);
        m_search->clear();
    }

    //unregister the previous help engine qch files
    if(!m_currentQchFileName.isEmpty())
    {
        const QString& fileNamespace = QHelpEngineCore::namespaceName(m_currentQchFileName);
        if(m_engine->registeredDocumentations().contains(fileNamespace))
            m_engine->unregisterDocumentation(m_currentQchFileName);
    }

    if (m_docNames.isEmpty())
    {
        m_contentWidget = nullptr;
        m_indexWidget = nullptr;
        return;
    }

    //initialize the Qt Help engine and provide the proper help collection file for the current backend
    //and for the currently selected documentation for this backend
    int index = m_documentationSelector->currentIndex();
    if (index < m_docPaths.size())
        m_currentQchFileName = m_docPaths.at(index);

    QString qhcFileName = m_currentQchFileName;
    qhcFileName.replace(QLatin1String("qch"), QLatin1String("qhc"));
    m_engine = new QHelpEngine(qhcFileName, this);
    /*if(!m_engine->setupData())
         qWarning() << "Couldn't setup QtHelp Engine: " << m_engine->error();*/

//     if(m_backend != QLatin1String("octave"))
      m_engine->setProperty("_q_readonly", QVariant::fromValue<bool>(true));

    //index widget
    m_indexWidget = m_engine->indexWidget();
    connect(m_indexWidget, &QHelpIndexWidget::linkActivated, this, &DocumentationPanelWidget::showUrl);

    //content widget
    m_contentWidget = m_engine->contentWidget();
    m_stackedWidget->addWidget(m_contentWidget);
    connect(m_contentWidget, &QHelpContentWidget::linkActivated, this, &DocumentationPanelWidget::showUrl);

    //search widget
    auto* completer = new QCompleter(m_indexWidget->model(), m_search);
    m_search->setCompleter(completer);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated), this, &DocumentationPanelWidget::returnPressed);

    // handle the URL scheme handler
    //m_webEngineView->page()->profile()->removeUrlScheme("qthelp");
    m_webEngineView->page()->profile()->removeAllUrlSchemeHandlers(); // remove previously installed scheme handler and then install new one
    m_webEngineView->page()->profile()->installUrlSchemeHandler("qthelp", new QtHelpSchemeHandler(m_engine));

    // register the compressed help file (qch)
    const QString& nameSpace = QHelpEngineCore::namespaceName(m_currentQchFileName);
    if(!m_engine->registeredDocumentations().contains(nameSpace))
    {
        if(m_engine->registerDocumentation(m_currentQchFileName))
            qDebug()<<"The documentation file " << m_currentQchFileName << " successfully registered.";
        else
            qWarning() << m_engine->error();
    }
}

void DocumentationPanelWidget::showUrl(const QUrl& url)
{
    m_webEngineView->load(url);
    m_stackedWidget->setCurrentIndex(0); //show the web engine view
}

QUrl DocumentationPanelWidget::url() const
{
    return m_webEngineView->url();
}

void DocumentationPanelWidget::returnPressed()
{
    const QString& input = m_search->text();

    if (input.isEmpty())
        return;

    contextSensitiveHelp(input);
}

void DocumentationPanelWidget::contextSensitiveHelp(const QString& keyword)
{
    qDebug() << "requested the documentation for the keyword " << keyword;

    //make sure first we show the web view in the stack widget
    m_stackedWidget->setCurrentIndex(0);

    m_indexWidget->filterIndices(keyword); // filter exactly, no wildcards
    m_indexWidget->activateCurrentItem(); // this internally emitts the QHelpIndexWidget::linkActivated signal

    // called in order to refresh and restore the index widget
    // otherwise filterIndices() filters the indices list, and then the index widget only contains the matched keywords
    m_indexWidget->filterIndices(QString());
}

void DocumentationPanelWidget::searchForward()
{
    m_matchCase->isChecked() ? m_webEngineView->findText(m_findText->text(), QWebEnginePage::FindCaseSensitively) :
                               m_webEngineView->findText(m_findText->text());
}

void DocumentationPanelWidget::searchBackward()
{
    m_matchCase->isChecked() ? m_webEngineView->findText(m_findText->text(), QWebEnginePage::FindCaseSensitively | QWebEnginePage::FindBackward) :
                               m_webEngineView->findText(m_findText->text(), QWebEnginePage::FindBackward);
}

void DocumentationPanelWidget::downloadResource(QWebEngineDownloadItem* resource)
{
    // default download directory is ~/Downloads on Linux
    m_webEngineView->page()->download(resource->url());
    resource->accept();

    KMessageBox::information(this, i18n("The file has been downloaded successfully at Downloads."), i18n("Download Successful"));

    disconnect(m_webEngineView->page()->profile(), &QWebEngineProfile::downloadRequested, this, &DocumentationPanelWidget::downloadResource);
}
