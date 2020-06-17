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
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QIcon>
#include <QPointer>
#include <QSplitter>
#include <QTabWidget>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>

DocumentationPanelWidget::DocumentationPanelWidget(Cantor::Session* session, QWidget* parent) :QWidget(parent), m_engine(nullptr), m_path(QString())
{
    addWidgets();
    setSession(session);
}

void DocumentationPanelWidget::setSession(Cantor::Session* session)
{
    m_session=session;
    /*if(session)
    {
        m_model=session->variableDataModel();
        if(m_table)
            m_table->setModel(m_model);
    }*/
}

void DocumentationPanelWidget::addWidgets()
{
    //QPointer<QSplitter> m_splitter;
    //QPointer<QTextBrowser> m_textBrowser;

    //m_engine = new QHelpEngine(QApplication::applicationDirPath() + QLatin1String("/documentation/maxima_help_collection.qhc"), this);
    m_engine = new QHelpEngine(QApplication::applicationDirPath() + QLatin1String("admin/documentation/maxima_help_collection.qhc"), this);
    m_engine->setupData();

    QByteArray helpData = m_engine->fileData(QUrl(QLatin1String("qthelp://org.kde.cantor/doc/maxima_7.html#SEC36")));

    if (!helpData.isEmpty())
        qDebug() << helpData;

    QPointer<QTabWidget> m_tabWidget = new QTabWidget;
    m_tabWidget->setMaximumWidth(1000);
    m_tabWidget->addTab(m_engine->indexWidget(), i18n("Index"));
    m_tabWidget->addTab(m_engine->contentWidget(), i18n("Contents"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_tabWidget, 1);

    // Connect to various signals and slots
}

void DocumentationPanelWidget::loadDocumentation()
{
   if(m_path.isEmpty()) {
        return;
    }

    const QStringList files = qchFiles();
    if(files.isEmpty()) {
        qWarning() << "Could not find QCH file in directory" << m_path;
        return;
    }

    for (const QString& fileName : files) {
        QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
        if (!fileNamespace.isEmpty() && !m_engine->registeredDocumentations().contains(fileNamespace)) {
            qDebug() << "Loading doc" << fileName << fileNamespace;
            if(!m_engine->registerDocumentation(fileName))
                qCritical() << "Error >> " << fileName << m_engine->error();
        }
    }
}

//search for QCH
QStringList DocumentationPanelWidget::qchFiles() const
{
    QStringList files;

    const QVector<QString> paths{ // test directories
        m_path,
        m_path + QLatin1String("/qch/"),
    };

    for (const auto& path : paths) {
        QDir d(path);
        if(path.isEmpty() || !d.exists()) {
            continue;
        }
        const auto fileInfos = d.entryInfoList(QDir::Files);
        for (const auto& file : fileInfos) {
            files << file.absoluteFilePath();
        }
    }
    if (files.isEmpty()) {
        qDebug() << "No QCH file found at all";
    }
    return files;
}

QIcon DocumentationPanelWidget::icon() const
{
    // determine backend name and then reurn it's icon
    //return QIcon::fromTheme(QStringLiteral("maxima"));
    return QIcon();
}

QString DocumentationPanelWidget::name() const
{
    // return the help name of the backend
    return i18n("Maxima Help");
}
