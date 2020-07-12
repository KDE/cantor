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
#include "session.h"

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
#include <QStandardPaths>
#include <QStackedWidget>
#include <QToolButton>
#include <QWebEngineProfile>
#include <QWebEngineUrlScheme>
#include <QWebEngineView>

DocumentationPanelWidget::DocumentationPanelWidget(Cantor::Session* session, QWidget* parent) :QWidget(parent), m_backend(QString())
{
    m_backend = session->backend()->name();
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

    QPushButton* home = new QPushButton(this);
    home->setIcon(QIcon::fromTheme(QLatin1String("go-home")));
    home->setToolTip(i18nc("@button go to contents page", "Go to the contents"));
    home->setEnabled(false);

    QComboBox* documentationSelector = new QComboBox(this);
    // iterate through the available docs for current backend, for example python may have matplotlib, scikitlearn etc
    documentationSelector->addItem(QIcon::fromTheme(session->backend()->icon()), m_backend);

    // Add a seperator
    QFrame *seperator = new QFrame(this);
    seperator->setFrameShape(QFrame::VLine);
    seperator->setFrameShadow(QFrame::Sunken);

    QStackedWidget* m_displayArea = new QStackedWidget(this);
    m_displayArea->addWidget(m_engine->contentWidget());

    QPushButton* findPage = new QPushButton(this);
    findPage->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));
    findPage->setToolTip(i18nc("@info:tooltip", "Find in text of current documentation page"));
    findPage->setEnabled(false);

    m_textBrowser = new QWebEngineView(this);

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


    // Add the Find in Page widget at the bottom, add all the widgets into a layout so that we can hide it
    QToolButton* hideButton = new QToolButton(this);
    hideButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
    hideButton->setToolTip(i18nc("@info:tooltip", "Close"));

    QLabel* label = new QLabel(this);
    label->setText(i18n("Find:"));

    QLineEdit* findText = new QLineEdit(this);
    findText->setPlaceholderText(i18nc("@info:placeholder", "Search..."));
    findText->setClearButtonEnabled(true);

    QToolButton* next = new QToolButton(this);
    next->setIcon(QIcon::fromTheme(QLatin1String("arrow-down")));
    next->setToolTip(i18nc("@info:tooltip", "Jump to next match"));

    QToolButton* previous = new QToolButton(this);
    previous->setIcon(QIcon::fromTheme(QLatin1String("arrow-up")));
    previous->setToolTip(i18nc("@info:tooltip", "Jump to previous match"));

    QToolButton* matchCase = new QToolButton(this);
    next->setIcon(QIcon::fromTheme(QLatin1String("format-text-superscript")));
    next->setToolTip(i18nc("@info:tooltip", "Match case sensitive"));

    // Create a layout
    QHBoxLayout* lout = new QHBoxLayout(this);
    lout->addWidget(hideButton);
    lout->addWidget(label);
    lout->addWidget(findText);
    lout->addWidget(next);
    lout->addWidget(previous);
    lout->addWidget(matchCase);

    QWidget* findPageWidgetContainer = new QWidget(this);
    findPageWidgetContainer->setLayout(lout);
    findPageWidgetContainer->hide();

    ////////////////////////////////////////////////////////
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

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(home, 0, 0);
    layout->addWidget(documentationSelector, 0, 1);
    layout->addWidget(m_search, 0, 2);
    layout->addWidget(seperator, 0, 3);
    layout->addWidget(findPage, 0, 4);
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
        }
        else
        {
            findPage->setEnabled(true);
            home->setEnabled(true);
        }
    });

    connect(home, &QPushButton::clicked, [=]{
        m_displayArea->setCurrentIndex(0);
    });

    connect(findPage, &QPushButton::clicked, [=]{
        layout->addWidget(findPageWidgetContainer, 2, 0, 3, 0);
        findPageWidgetContainer->show();
    });

    connect(m_engine->contentWidget(), &QHelpContentWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);
    connect(m_index, &QHelpIndexWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);
    connect(m_index, &QHelpIndexWidget::activated, this, &DocumentationPanelWidget::refreshIndexWidget);
    connect(m_search, &QLineEdit::returnPressed, this, &DocumentationPanelWidget::returnPressed);
    connect(m_search->completer(), QOverload<const QModelIndex&>::of(&QCompleter::activated), this, &DocumentationPanelWidget::returnPressed);

    connect(hideButton, &QToolButton::clicked, this, [=]{
        findPageWidgetContainer->hide();
    });
    /*connect(matchCase, &QAbstractButton::toggled, this, &DocumentationPanelWidget::emitDataChanged);
    connect(findText, &QLineEdit::returnPressed, this, &DocumentationPanelWidget::searchNext);
    connect(previous, &QToolButton::clicked, this, &DocumentationPanelWidget::searchPrevious);
    connect(next, &QToolButton::clicked, this, &DocumentationPanelWidget::searchNext);*/

    setSession(session);
}

DocumentationPanelWidget::~DocumentationPanelWidget()
{
    delete m_engine;
    delete m_textBrowser;
    delete m_displayArea;
    delete m_index;
    delete m_search;
}

void DocumentationPanelWidget::setSession(Cantor::Session* session)
{
    m_session = session;
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

void DocumentationPanelWidget::refreshIndexWidget()
{
    //QHelpIndexWidget* index = m_engine->indexWidget();
    m_index->filterIndices(QString());
    m_index->activateCurrentItem();
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
