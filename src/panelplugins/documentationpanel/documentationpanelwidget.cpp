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
#include <QPushButton>
#include <QSplitter>
#include <QStandardPaths>
#include <QTabWidget>
#include <QWebEngineView>

DocumentationPanelWidget::DocumentationPanelWidget(Cantor::Session* session, QWidget* parent) :QWidget(parent), m_backend(QString())
{
    m_backend = session->backend()->name();
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + m_backend + QLatin1String("/help.qhc"));
    m_engine = new QHelpEngine(fileName, this);

    if(!m_engine->setupData())
    {
        qWarning() << "Couldn't setup QtHelp Engine";
        qWarning() << m_engine->error();
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

    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->setMovable(true);
    tabWidget->setElideMode(Qt::ElideRight);

    // Add different tabs to the widget
    tabWidget->addTab(m_engine->contentWidget(), i18n("Contents"));
    tabWidget->addTab(m_engine->indexWidget(), i18n("Index"));
    tabWidget->addTab(container, i18n("Search"));

    m_textBrowser = new QWebEngineView(this);

    // set initial page contents, otherwise page is blank
    QByteArray contents;

    if(m_backend == QLatin1String("Maxima"))
    {
        contents = m_engine->fileData(QUrl(QLatin1String("qthelp://org.kde.cantor/doc/maxima.html#SEC_Top")));
    }
    else if(m_backend == QLatin1String("Octave"))
    {
        contents = m_engine->fileData(QUrl(QLatin1String("qthelp://org.octave.interpreter-1.0/doc/octave.html/index.html")));
    }

    m_textBrowser->setContent(contents, QLatin1String("text/html;charset=UTF-8"));
    m_textBrowser->show();

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(tabWidget);
    splitter->addWidget(m_textBrowser);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    //TODO QHelpIndexWidget::linkActivated is obsolete, use QHelpIndexWidget::documentActivated instead
    connect(m_engine->contentWidget(), &QHelpContentWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);
    connect(m_engine->indexWidget(), &QHelpIndexWidget::linkActivated, this, &DocumentationPanelWidget::displayHelp);

    setSession(session);
}

DocumentationPanelWidget::~DocumentationPanelWidget()
{
    delete m_engine;
    delete m_textBrowser;
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

    const QModelIndex index = m_engine->indexWidget()->currentIndex();
    const QString indexText = index.data(Qt::DisplayRole).toString();
    qDebug() << indexText << "index pressed";
}

void DocumentationPanelWidget::doSearch(const QString& str)
{
    Q_UNUSED(str)
}

void DocumentationPanelWidget::contextSensitiveHelp(const QString& keyword)
{
    qDebug() << "Context sensitive help for " << keyword;

    QHelpIndexWidget* index = m_engine->indexWidget();
    index->filterIndices(keyword, keyword); // filter exactly, no wildcards
    index->activateCurrentItem(); // this internally emitts the QHelpIndexWidget::linkActivated signal

    loadDocumentation();
}

void DocumentationPanelWidget::loadDocumentation()
{
    const QString backend = backendName();
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backend + QLatin1String("/help.qch"));
    const QString nameSpace = QHelpEngineCore::namespaceName(fileName);

    //if(nameSpace.isEmpty() || !m_engine->registeredDocumentations().contains(nameSpace))
    //{
        if(!m_engine->registerDocumentation(fileName))
            qWarning() << m_engine->error();
    //}
}

QString DocumentationPanelWidget::backendName() const
{
    return m_backend;
}
