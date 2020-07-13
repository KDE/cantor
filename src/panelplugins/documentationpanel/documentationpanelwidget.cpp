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
    Copyright (C) 2020 Shubham <aryan100jangid@gmail.com>
 */

#include "cantor_macros.h"
#include "documentationpanelplugin.h"

#include <KLocalizedString>

#include <QCompleter>
#include <QComboBox>
#include <QDebug>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QPushButton>
#include <QShortcut>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QToolButton>
#include <QWebEngineProfile>
#include <QWebEngineUrlScheme>
#include <QWebEngineView>

DocumentationPanelWidget::DocumentationPanelWidget(const QString& backend, const QString& backendIcon, QWidget* parent) :QWidget(parent)
{
    m_backend = backend;
    m_icon = backendIcon;

    const QString& fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + m_backend + QLatin1String("/help.qhc"));

    m_engine = new QHelpEngine(fileName, this);

    if(!m_engine->setupData())
    {
        qWarning() << "Couldn't setup QtHelp Engine";
        qWarning() << m_engine->error();
    }

    if(m_backend != QLatin1String("Octave"))
    {
      m_engine->setProperty("_q_readonly", QVariant::fromValue<bool>(true));
    }

    loadDocumentation();

    m_textBrowser = new QWebEngineView(this);

    // Register custom scheme handler for qthelp:// scheme
    static bool qthelpRegistered = false;

    if(!qthelpRegistered)
    {
        QWebEngineUrlScheme qthelp("qthelp");
        QWebEngineUrlScheme::registerScheme(qthelp);
        m_textBrowser->page()->profile()->installUrlSchemeHandler("qthelp", new QtHelpSchemeHandler(m_engine));
        qthelpRegistered = true;
    }

    // set initial page contents, otherwise page is blank
    if(m_backend == QLatin1String("Maxima"))
    {
        m_textBrowser->load(QUrl(QLatin1String("qthelp://org.kde.cantor/doc/maxima.html")));
        m_textBrowser->show();
    }
    else if(m_backend == QLatin1String("Octave"))
    {
        m_textBrowser->load(QUrl(QLatin1String("qthelp://org.octave.interpreter-1.0/doc/octave.html/index.html")));
        m_textBrowser->show();
    }

    QPushButton* home = new QPushButton(this);
    home->setIcon(QIcon::fromTheme(QLatin1String("go-home")));
    home->setToolTip(i18nc("@button go to contents page", "Go to the contents"));
    home->setEnabled(false);

    QComboBox* documentationSelector = new QComboBox(this);
    // iterate through the available docs for current backend, for example python may have matplotlib, scikitlearn etc
    documentationSelector->addItem(QIcon::fromTheme(m_icon), m_backend);

    // Add a seperator
    QFrame *seperator = new QFrame(this);
    seperator->setFrameShape(QFrame::VLine);
    seperator->setFrameShadow(QFrame::Sunken);

    QStackedWidget* m_displayArea = new QStackedWidget(this);
    m_displayArea->addWidget(m_engine->contentWidget());

    QPushButton* findPage = new QPushButton(this);
    findPage->setEnabled(false);
    findPage->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    findPage->setToolTip(i18nc("@info:tooltip", "Find in text of current documentation page"));
    findPage->setShortcut(QKeySequence(/*Qt::CTRL + */Qt::Key_F3));

    QPushButton* resetZoom = new QPushButton(this);
    resetZoom->setEnabled(false);
    resetZoom->setIcon(QIcon::fromTheme(QLatin1String("zoom-fit-best")));
    resetZoom->setToolTip(i18nc("@info:tooltip", "Reset zoom level to 100%"));
    //resetZoom->setShortcut(QKeySequence(/*Qt::CTRL + */Qt::Key_F3));

    m_displayArea->addWidget(m_textBrowser);

    /* Adding the index widget to implement the logic for context sensitive help
     * This widget would be NEVER shown*/
    m_index = m_engine->indexWidget();
    m_displayArea->addWidget(m_index);

    // real time searcher
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(i18nc("@info:placeholder", "Search through keywords..."));
    m_search->setClearButtonEnabled(true);

    m_search->setCompleter(new QCompleter(m_index->model(), m_search));
    m_search->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_search->completer()->setCaseSensitivity(Qt::CaseInsensitive);

    // Add zoom in, zoom out behaviour on SHIFT++ and SHIFT--
    auto zoomIn = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Plus), this);
    zoomIn->setContext(Qt::WidgetWithChildrenShortcut);
    connect(zoomIn, &QShortcut::activated, this, [=]{
        m_textBrowser->setZoomFactor(m_textBrowser->zoomFactor() + 0.1);
    });

    auto zoomOut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Minus), this);
    zoomOut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(zoomOut, &QShortcut::activated, this, [=]{
        m_textBrowser->setZoomFactor(m_textBrowser->zoomFactor() - 0.1);
    });

    // Add the Find in Page widget at the bottom, add all the widgets into a layout so that we can hide it
    QToolButton* hideButton = new QToolButton(this);
    hideButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
    hideButton->setToolTip(i18nc("@info:tooltip", "Close"));

    QLabel* label = new QLabel(this);
    label->setText(i18n("Find:"));

    m_findText = new QLineEdit(this);
    m_findText->setPlaceholderText(i18nc("@info:placeholder", "Search..."));
    m_findText->setClearButtonEnabled(true);

    QToolButton* next = new QToolButton(this);
    next->setIcon(QIcon::fromTheme(QLatin1String("go-down-search")));
    next->setToolTip(i18nc("@info:tooltip", "Jump to next match"));

    QToolButton* previous = new QToolButton(this);
    previous->setIcon(QIcon::fromTheme(QLatin1String("go-up-search")));
    previous->setToolTip(i18nc("@info:tooltip", "Jump to previous match"));

    m_matchCase = new QToolButton(this);
    m_matchCase->setIcon(QIcon::fromTheme(QLatin1String("format-text-superscript")));
    m_matchCase->setToolTip(i18nc("@info:tooltip", "Match case sensitive"));
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

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(home, 0, 0);
    layout->addWidget(documentationSelector, 0, 1);
    layout->addWidget(m_search, 0, 2);
    layout->addWidget(seperator, 0, 3);
    layout->addWidget(findPage, 0, 4);
    layout->addWidget(resetZoom, 0, 5);
    layout->addWidget(m_displayArea, 1, 0, 2, 0);

    //TODO QHelpIndexWidget::linkActivated is obsolete, use QHelpIndexWidget::documentActivated instead
    // display the documentation browser whenever contents are clicked
    connect(m_engine->contentWidget(), &QHelpContentWidget::linkActivated, [=](){
        m_displayArea->setCurrentIndex(1);
    });

    connect(this, &DocumentationPanelWidget::activateBrowser, [=]{
        m_textBrowser->hide();
        m_displayArea->setCurrentIndex(1);
    });

    connect(m_displayArea, &QStackedWidget::currentChanged, [=]{
        //disable Home and Search in Page buttons when stackwidget shows contents widget, enable when shows web browser
        if(m_displayArea->currentIndex() != 1) //0->contents 1->browser
        {
            findPage->setEnabled(false);
            home->setEnabled(false);
            resetZoom->setEnabled(false);
        }
        else
        {
            findPage->setEnabled(true);
            home->setEnabled(true);
            resetZoom->setEnabled(true);
        }
    });

    connect(home, &QPushButton::clicked, [=]{
        m_displayArea->setCurrentIndex(0);
    });

    connect(resetZoom, &QPushButton::clicked, [=]{
        m_textBrowser->setZoomFactor(1.0);
    });

    connect(m_engine->contentWidget(), &QHelpContentWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);
    connect(m_index, &QHelpIndexWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);
    connect(m_search, &QLineEdit::returnPressed, this, &DocumentationPanelWidget::returnPressed);
    connect(m_search->completer(), QOverload<const QModelIndex&>::of(&QCompleter::activated), this, &DocumentationPanelWidget::returnPressed);

    // connect statements for Find in Page text widget
    connect(findPage, &QPushButton::clicked, [=]{
        layout->setRowStretch(1, 1);
        layout->addWidget(findPageWidgetContainer, 2, 0, 2, 0, Qt::AlignBottom);
        findPageWidgetContainer->show();
        m_findText->clear();
        m_findText->setFocus();
    });

    connect(hideButton, &QToolButton::clicked, this, [=]{
        layout->removeWidget(findPageWidgetContainer);
        findPageWidgetContainer->hide();
        m_textBrowser->findText(QString()); // this clears up the selected text
    });

    connect(m_findText, &QLineEdit::returnPressed, this, &DocumentationPanelWidget::searchForward);
    connect(m_findText, &QLineEdit::textEdited, this, &DocumentationPanelWidget::searchForward); // for highlighting found string in real time
    connect(next, &QToolButton::clicked, this, &DocumentationPanelWidget::searchForward);
    connect(previous, &QToolButton::clicked, this, &DocumentationPanelWidget::searchBackward);
    connect(m_matchCase, &QAbstractButton::toggled, this, &DocumentationPanelWidget::searchForward);
    connect(m_matchCase, &QAbstractButton::toggled, this, [=]{
        m_textBrowser->findText(QString());
        searchForward();
    });
}

DocumentationPanelWidget::~DocumentationPanelWidget()
{
    delete m_engine;
    delete m_textBrowser;
    delete m_displayArea;
    delete m_index;
    delete m_search;
    delete m_findText;
    delete m_matchCase;
}

void DocumentationPanelWidget::setBackend(const QString& backend)
{
    m_backend = backend;
}

void DocumentationPanelWidget::setBackendIcon(const QString& icon)
{
    m_icon = icon;
}

void DocumentationPanelWidget::displayHelp(const QUrl& url)
{
    m_textBrowser->load(url);
    m_textBrowser->show();
}

void DocumentationPanelWidget::returnPressed()
{
    const QString& input = m_search->text();

    if (input.isEmpty())
        return;

    /*auto model = m_index->model();
    //auto model = m_engine->indexModel();

    for(int row = 0; row < model->rowCount(); ++row)
    {
        auto keyword = model->index(row, 0);
        if(keyword.data().toString() == input)
        {

            break;
        }
    }*/

    contextSensitiveHelp(input);
}

void DocumentationPanelWidget::contextSensitiveHelp(const QString& keyword)
{
    // First make sure we have display browser as the current widget on the QStackedWidget
    emit activateBrowser();

    m_index->filterIndices(keyword); // filter exactly, no wildcards
    m_index->activateCurrentItem(); // this internally emitts the QHelpIndexWidget::linkActivated signal
}

void DocumentationPanelWidget::searchForward()
{
    m_matchCase->isChecked() ? m_textBrowser->findText(m_findText->text(), QWebEnginePage::FindCaseSensitively) :
                               m_textBrowser->findText(m_findText->text());
}

void DocumentationPanelWidget::searchBackward()
{
    m_matchCase->isChecked() ? m_textBrowser->findText(m_findText->text(), QWebEnginePage::FindCaseSensitively | QWebEnginePage::FindBackward) :
                               m_textBrowser->findText(m_findText->text(), QWebEnginePage::FindBackward);
}

void DocumentationPanelWidget::loadDocumentation()
{
    const QString& backend = backendName();
    const QString& fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backend + QLatin1String("/help.qch"));
    const QString& nameSpace = QHelpEngineCore::namespaceName(fileName);

    if(nameSpace.isEmpty() || !m_engine->registeredDocumentations().contains(nameSpace))
    {
        if(!m_engine->registerDocumentation(fileName))
            qWarning() << m_engine->error();
    }
}

QString DocumentationPanelWidget::backendName() const
{
    return m_backend;
}
