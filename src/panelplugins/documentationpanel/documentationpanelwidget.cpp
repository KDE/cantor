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
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextBrowser>
#include <QUrl>

DocumentationPanelWidget::DocumentationPanelWidget(QWidget* parent) :QWidget(parent), m_engine(nullptr)
{
    const QString backendName = QLatin1String("maxima");
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backendName + QLatin1String("/help.qhc"));
    m_engine = new QHelpEngine(fileName, this);

    if( !m_engine->setupData() )
    {
        qWarning() << "Couldn't setup QtHelp Collection file";
    }

    QPointer<QTabWidget> tabWidget = new QTabWidget(this);
    tabWidget->setMaximumWidth(1000);
    tabWidget->setMinimumWidth(500);
    tabWidget->setMovable(true);
    tabWidget->setElideMode(Qt::ElideRight);
    tabWidget->addTab(m_engine->indexWidget(), i18n("Index"));
    tabWidget->addTab(m_engine->contentWidget(), i18n("Contents"));


    QPointer<QTextBrowser> textBrowser = new QTextBrowser(parentWidget());

    connect(m_engine->contentWidget(), SIGNAL(linkActivated(QUrl)), textBrowser, SLOT(setSource(QUrl)));
    connect(m_engine->indexWidget(), SIGNAL(linkActivated(QUrl, QString)), textBrowser, SLOT(setSource(QUrl)));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget, 1);
    layout->addWidget(textBrowser, 2);

    loadDocumentation();
}

void DocumentationPanelWidget::loadDocumentation()
{
    const QString backendName = QLatin1String("maxima");
    const QString fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("documentation/") + backendName + QLatin1String("/help.qch"));
    m_engine->registerDocumentation(fileName);
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
