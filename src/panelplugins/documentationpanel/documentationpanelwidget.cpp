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

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QHBoxLayout>
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QIcon>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSplitter>
#include <QStandardPaths>
#include <QTabWidget>
#include <QUrl>
#include <QWebEngineView>
#include <QWidget>

DocumentationPanelWidget::DocumentationPanelWidget(Cantor::Session* session, QWidget* parent) :QWidget(parent), m_session(nullptr), m_engine(nullptr), m_textBrowser(nullptr), m_tabWidget(nullptr), m_splitter(nullptr), m_backend(QString())
{
    m_backend = session->backend()->name();
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + m_backend + QLatin1String("/help.qhc"));
    m_engine = new QHelpEngine(fileName, this);

    if(!m_engine->setupData())
    {
        qWarning() << "Couldn't setup QtHelp Engine";
        delete m_engine;
        delete m_textBrowser;
        delete m_tabWidget;
        delete m_splitter;
    }

    loadDocumentation();

    // create  a container for Search tab
    QWidget* container = new QWidget(this);
    QHBoxLayout* clayout = new QHBoxLayout(this);
    container->setLayout(clayout);

    QLineEdit* input = new QLineEdit(this);
    QPushButton* search = new QPushButton(i18n("Search"), this);
    clayout->addWidget(input);
    clayout->addWidget(search);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setMovable(true);
    m_tabWidget->setElideMode(Qt::ElideRight);

    // Add different tabs to the widget
    m_tabWidget->addTab(m_engine->contentWidget(), i18n("Contents"));
    m_tabWidget->addTab(m_engine->indexWidget(), i18n("Index"));
    m_tabWidget->addTab(container, i18n("Search"));

    m_textBrowser = new QWebEngineView(this);
    const QByteArray contents = m_engine->fileData(QUrl(QLatin1String("qthelp://org.kde.cantor/doc/maxima.html#SEC_Top"))); // set initial page contents
    m_textBrowser->setContent(contents, QLatin1String("text/html;charset=UTF-8"));
    m_textBrowser->show();

    //const QByteArray contents = m_engine->fileData(QUrl(QLatin1String("qthelp://org.kde.cantor/doc/figures/plotting1.png")));
    //m_textBrowser->setContent(contents, QLatin1String("image/png;charset=UTF-8"));

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(m_tabWidget);
    m_splitter->addWidget(m_textBrowser);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_splitter);

    connect(m_engine->contentWidget(), &QHelpContentWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);
    connect(m_engine->indexWidget(), &QHelpIndexWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);
    //connect(search, SIGNAL(clicked(bool)), this, SLOT(doSearch(QString)));

    setSession(session);
}

void DocumentationPanelWidget::setSession(Cantor::Session* session)
{
    m_session = session;
}

void DocumentationPanelWidget::displayHelp(const QUrl& url)
{
    const QByteArray contents = m_engine->fileData(url);
    m_textBrowser->setContent(contents, QLatin1String("text/html;charset=UTF-8"));
    m_textBrowser->show();

    qDebug() << url;
    //display the actual keyword contents, not the header topic
    const QModelIndex index = m_engine->indexWidget()->currentIndex();
    const QString indexText = index.data(Qt::DisplayRole).toString();
    qDebug() << indexText << "index pressed";
}

void DocumentationPanelWidget::doSearch(const QString& str)
{
    // perform searching of the string passed
    Q_UNUSED(str)
}

void DocumentationPanelWidget::contextSensitiveHelp(const QString& keyword)
{
    qDebug() << "Context sensitive help for " << keyword;

    QHelpIndexWidget* const index = m_engine->indexWidget();
    index->filterIndices(keyword); // filter exactly, no wildcards
    index->activateCurrentItem(); // this internally emitts the QHelpIndexWidget::linkActivated signal

    loadDocumentation();
}

void DocumentationPanelWidget::loadDocumentation()
{
    const QString backend = backendName();
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backend + QLatin1String("/help.qch"));
    m_engine->registerDocumentation(fileName);
}

void DocumentationPanelWidget::unloadDocumentation()
{
    //Call this function when the user changes the current backend
    m_engine->unregisterDocumentation(QLatin1String("org.kde.cantor"));
}

QString DocumentationPanelWidget::backendName() const
{
    return m_backend;
}
