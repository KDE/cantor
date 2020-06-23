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
#include <QDir>
#include <QHBoxLayout>
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QIcon>
#include <QPointer>
#include <QSplitter>
#include <QStandardPaths>
#include <QTabWidget>
#include <QUrl>
#include <QWebEngineView>

DocumentationPanelWidget::DocumentationPanelWidget(QWidget* parent) :QWidget(parent), m_engine(nullptr), m_textBrowser(nullptr), m_tabWidget(nullptr), m_splitter(nullptr)
{
    const QString backendName = QLatin1String("maxima");
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backendName + QLatin1String("/help.qhc"));
    m_engine = new QHelpEngine(fileName, this);

    if(!m_engine->setupData())
    {
        qWarning() << "Couldn't setup QtHelp Engine";
        delete m_engine;
        delete m_textBrowser;
        delete m_tabWidget;
        delete m_splitter;
    }

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setMaximumWidth(1000);
    m_tabWidget->setMinimumWidth(500);
    m_tabWidget->setMovable(true);
    m_tabWidget->setElideMode(Qt::ElideRight);
    m_tabWidget->addTab(m_engine->indexWidget(), i18n("Index"));
    m_tabWidget->addTab(m_engine->contentWidget(), i18n("Contents"));

    // later add properties like contextmenu event, keyevent, mousevent to the browser
    m_textBrowser = new QWebEngineView(this);
    QByteArray contents = m_engine->fileData(QUrl(QLatin1String("qthelp://org.kde.cantor/doc/maxima.html#SEC_Top"))); // set initial page contents
    m_textBrowser->setContent(contents, QLatin1String("text/html;charset=UTF-8"));
    m_textBrowser->show();

    connect(m_engine->contentWidget(), SIGNAL(linkActivated(QUrl)), this, SLOT(displayHelp(QUrl)));
    connect(m_engine->indexWidget(), SIGNAL(linkActivated(QUrl, QString)), this, SLOT(displayHelp(QUrl)));

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(m_tabWidget);
    m_splitter->addWidget(m_textBrowser);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_splitter);

    loadDocumentation();
}

void DocumentationPanelWidget::displayHelp(const QUrl& url)
{
    QByteArray contents = m_engine->fileData(url);
    m_textBrowser->setContent(contents, QLatin1String("text/html;charset=UTF-8"));
    m_textBrowser->show();
}

void DocumentationPanelWidget::loadDocumentation()
{
    //1. Get the backend name
    //2. Load their documentation

    const QString backendName = QLatin1String("maxima");
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backendName + QLatin1String("/help.qch"));
    m_engine->registerDocumentation(fileName);
}

void DocumentationPanelWidget::unloadDocumentation()
{
    //1. Get the backend name
    //2. Unload their documentation
    //Call this function when the user changes the current backend

    const QString backendName = QLatin1String("maxima");
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backendName + QLatin1String("/help.qch"));
    m_engine->unregisterDocumentation(fileName);
}

QIcon DocumentationPanelWidget::icon() const
{
    // return backend's icon
    return QIcon();
}

QString DocumentationPanelWidget::name() const
{
    // return backend's name
    return QString(QLatin1String("maxima"));
}
