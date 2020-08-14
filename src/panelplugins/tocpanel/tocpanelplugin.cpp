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

#include "tocpanelplugin.h"

#include <cassert>
#include <QListView>
#include <QDebug>
#include <QWidget>

TableOfContentPanelPlugin::TableOfContentPanelPlugin(QObject* parent, const QList<QVariant>& args): Cantor::PanelPlugin(parent),
    m_mainWidget(nullptr)
{
    Q_UNUSED(args);
}

TableOfContentPanelPlugin::~TableOfContentPanelPlugin()
{
    if (m_mainWidget)
    {
        m_mainWidget->deleteLater();
    }
}

QWidget* TableOfContentPanelPlugin::widget()
{
    if (!m_mainWidget)
        constructMainWidget();

    return m_mainWidget;
}

void TableOfContentPanelPlugin::connectToShell(QObject* cantorShell)
{
    connect(cantorShell, SIGNAL(hierarchyChanged(QStringList, QStringList, QList<int>)), this, SLOT(handleHierarchyChanges(QStringList, QStringList, QList<int>)));
    connect(this, SIGNAL(requestScrollToHierarchyEntry(QString)), cantorShell, SIGNAL(requestScrollToHierarchyEntry(QString)));
    connect(cantorShell, SIGNAL(hierarhyEntryNameChange(QString, QString, int)), this, SLOT(handleHierarhyEntryNameChange(QString, QString, int)));
}

bool TableOfContentPanelPlugin::showOnStartup()
{
    return false;
}

void TableOfContentPanelPlugin::handleDoubleClicked(const QModelIndex& index)
{
    qDebug() << "TableOfContentPanelPlugin::handleDoubleClicked";
    const QString& searchStringIndex = m_hierarchyPositionStringList[index.row()];
    emit requestScrollToHierarchyEntry(searchStringIndex);
}

void TableOfContentPanelPlugin::constructMainWidget()
{
    QListView* view = new QListView(m_mainWidget);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setModel(&m_model);

    connect(view, &QListView::doubleClicked, this, &TableOfContentPanelPlugin::handleDoubleClicked);

    m_mainWidget = view;
}

void TableOfContentPanelPlugin::restoreState(const Cantor::PanelPlugin::State& state)
{
    if (state.inners.size() == 2)
    {
        m_model.setStringList(state.inners[0].toStringList());
        m_hierarchyPositionStringList = state.inners[1].toStringList();
    }
}

Cantor::PanelPlugin::State TableOfContentPanelPlugin::saveState()
{
    Cantor::PanelPlugin::State state;

    state.inners.append(m_model.stringList());
    state.inners.append(m_hierarchyPositionStringList);

    return state;
}

void TableOfContentPanelPlugin::handleHierarchyChanges(QStringList names, QStringList searchStrings, QList<int> depths)
{
    QStringList fullNames;

    assert(names.size() == searchStrings.size() && names.size() == depths.size());

    int size = names.size();
    for (int i = 0; i < size; i++)
        fullNames.append(QString::fromLatin1("  ").repeated(depths[i]) + searchStrings[i] + QLatin1String(" ") + names[i]);

    m_model.setStringList(fullNames);
    m_hierarchyPositionStringList = searchStrings;
}

void TableOfContentPanelPlugin::handleHierarhyEntryNameChange(QString name, QString searchString, int deapth)
{
    int size = m_hierarchyPositionStringList.size();
    for (int i = 0; i < size; i++)
    {
        if (m_hierarchyPositionStringList[i] == searchString)
        {
            QModelIndex index = m_model.index(i);
            const QString& fullName = QString::fromLatin1("  ").repeated(deapth) + searchString + QLatin1String(" ") + name;
            m_model.setData(index, fullName);
        }
    }
}

K_PLUGIN_FACTORY_WITH_JSON(tocpanelplugin, "tocpanelplugin.json", registerPlugin<TableOfContentPanelPlugin>();)
#include "tocpanelplugin.moc"
