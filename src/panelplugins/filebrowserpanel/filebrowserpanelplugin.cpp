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
    Copyright (C) 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#include "filebrowserpanelplugin.h"

#include <QFileSystemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QModelIndex>
#include <QStandardItem>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>

#include <KParts/ReadOnlyPart>

QFileSystemModel* FileBrowserPanelPlugin::model = nullptr;

FileBrowserPanelPlugin::FileBrowserPanelPlugin(QObject* parent, const QList<QVariant>& args): Cantor::PanelPlugin(parent),
    m_view(nullptr)
{
    Q_UNUSED(args);

    KParts::ReadOnlyPart* part = dynamic_cast<KParts::ReadOnlyPart*>(parent->parent());
    if (part)
        m_dirRoot = QFileInfo(part->url().toLocalFile()).absoluteDir().absolutePath();
    else
        m_dirRoot = QDir::currentPath();
}

FileBrowserPanelPlugin::~FileBrowserPanelPlugin()
{
    if (m_view)
        m_view->deleteLater();;
}

QWidget* FileBrowserPanelPlugin::widget()
{
    if (!m_view)
    {
        if (!model)
        {
            model = new QFileSystemModel();
            model->setRootPath(QDir::currentPath());
        }

        m_view = new QTreeView();
        m_view->setModel(model);
        m_view->setRootIndex(model->index(m_dirRoot));
        // First column is name with the dir tree
        for (int i = 1; i < model->columnCount(); i++)
            m_view->setColumnHidden(i, true);
        m_view->header()->hide();

        connect(m_view, &QTreeView::doubleClicked, this, &FileBrowserPanelPlugin::handleDoubleClicked);
        connect(this, SIGNAL(requestOpenWorksheet(QUrl)), parent()->parent(), SIGNAL(requestOpenWorksheet(QUrl)));
    }

    return m_view;
}

bool FileBrowserPanelPlugin::showOnStartup()
{
    return false;
}

void FileBrowserPanelPlugin::handleDoubleClicked(const QModelIndex& index)
{
    if (model->isDir(index))
        return;

    QVariant data = model->data(index, QFileSystemModel::FilePathRole);
    if (data.isValid() && data.type() == QVariant::String)
    {
        const QString& filename = data.value<QString>();
        const QUrl& url = QUrl::fromLocalFile(filename);

        if (filename.endsWith(QLatin1String(".cws")) || filename.endsWith(QLatin1String(".ipynb")))
            emit requestOpenWorksheet(url);
        else
            QDesktopServices::openUrl(url);
    }
}

K_PLUGIN_FACTORY_WITH_JSON(filebrowserpanelplugin, "filebrowserpanelplugin.json", registerPlugin<FileBrowserPanelPlugin>();)
#include "filebrowserpanelplugin.moc"
