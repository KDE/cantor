/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#include "tocpanelplugin.h"

#include <cassert>
#include <QListView>
#include <QDebug>
#include <QWidget>

#include <KPluginFactory>

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
    Q_EMIT requestScrollToHierarchyEntry(searchStringIndex);
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
    m_hierarchyPositionStringList = std::move(searchStrings);
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
