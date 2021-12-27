/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
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
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QLineEdit>
#include <QComboBox>

#include <KLocalizedString>
#include <KParts/ReadOnlyPart>

FileBrowserPanelPlugin::FileBrowserPanelPlugin(QObject* parent, const QList<QVariant>& args): Cantor::PanelPlugin(parent),
    m_mainWidget(nullptr), m_treeview(nullptr), m_pathEdit(nullptr), m_filterCombobox(nullptr)
{
    Q_UNUSED(args);

    auto* part = dynamic_cast<KParts::ReadOnlyPart*>(parent->parent());
    QString baseRootDir;
    if (part && !part->url().isEmpty())
        baseRootDir = QFileInfo(part->url().toLocalFile()).absoluteDir().absolutePath();
    else
        baseRootDir = QDir::currentPath();
    m_rootDirsHistory.push_back(baseRootDir);
}

FileBrowserPanelPlugin::~FileBrowserPanelPlugin()
{
    if (m_mainWidget)
    {
        m_mainWidget->deleteLater();
        m_treeview = nullptr;
        m_pathEdit = nullptr;
        m_filterCombobox = nullptr;
        m_model->deleteLater();
    }
}

QWidget* FileBrowserPanelPlugin::widget()
{
    if (!m_mainWidget)
    {
        m_model = new QFileSystemModel();
        m_model->setRootPath(m_rootDirsHistory.last());
        constructMainWidget();
    }

    return m_mainWidget;
}

void FileBrowserPanelPlugin::connectToShell(QObject* cantorShell)
{
    connect(this, SIGNAL(requestOpenWorksheet(QUrl)), cantorShell, SLOT(load(QUrl)));
}

bool FileBrowserPanelPlugin::showOnStartup()
{
    return false;
}

void FileBrowserPanelPlugin::handleDoubleClicked(const QModelIndex& index)
{
    QVariant data = m_model->data(index, QFileSystemModel::FilePathRole);
    if (data.isValid() && data.type() == QVariant::String)
    {
        const QString& filename = data.value<QString>();
        if (m_model->isDir(index))
        {
            moveFileBrowserRoot(filename);
        }
        else
        {
            const QUrl& url = QUrl::fromLocalFile(filename);
            if (filename.endsWith(QLatin1String(".cws")) || filename.endsWith(QLatin1String(".ipynb")))
                emit requestOpenWorksheet(url);
            else
                QDesktopServices::openUrl(url);
        }
    }
}

void FileBrowserPanelPlugin::constructMainWidget()
{
    m_mainWidget = new QWidget();

    m_treeview = new QTreeView(m_mainWidget);
    m_treeview->setModel(m_model);
    m_treeview->setRootIndex(m_model->index(m_rootDirsHistory.last()));
    m_treeview->setExpandsOnDoubleClick(false);
    connect(m_treeview, &QTreeView::doubleClicked, this, &FileBrowserPanelPlugin::handleDoubleClicked);

    // First column is name with the dir tree
    // Show only the first column
    for (int i = 1; i < m_model->columnCount(); i++)
        m_treeview->setColumnHidden(i, true);
    m_treeview->header()->hide();
    m_treeview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget* buttonContainer = new QWidget(m_mainWidget);

    QPushButton* dirUpButton = new QPushButton(QIcon::fromTheme(QLatin1String("go-up")), QString(), buttonContainer);
    dirUpButton->setMinimumSize(40, 40);
    connect(dirUpButton, &QPushButton::clicked, this, &FileBrowserPanelPlugin::dirUpButtonHandle);

    QPushButton* homeButton = new QPushButton(QIcon::fromTheme(QLatin1String("go-home")), QString(), buttonContainer);
    homeButton->setMinimumSize(40, 40);
    connect(homeButton, &QPushButton::clicked, this, &FileBrowserPanelPlugin::homeButtonHandle);

    QPushButton* dirPreviousButton = new QPushButton(QIcon::fromTheme(QLatin1String("go-previous")), QString(), buttonContainer);
    dirPreviousButton->setMinimumSize(40, 40);
    connect(dirPreviousButton, &QPushButton::clicked, this, &FileBrowserPanelPlugin::dirPreviousButtonHandle);

    QPushButton* dirNextButton = new QPushButton(QIcon::fromTheme(QLatin1String("go-next")), QString(), buttonContainer);
    dirNextButton->setMinimumSize(40, 40);
    connect(dirNextButton, &QPushButton::clicked, this, &FileBrowserPanelPlugin::dirNextButtonHandle);

    m_pathEdit = new QLineEdit(m_rootDirsHistory.last(), buttonContainer);
    connect(m_pathEdit, &QLineEdit::returnPressed, this, &FileBrowserPanelPlugin::setNewRootPath);
    m_pathEdit->setMinimumHeight(40);
    m_pathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_filterCombobox = new QComboBox(buttonContainer);
    m_filterCombobox->addItem(i18n("Cantor files"), QLatin1String("*.cws")); //Default value
    m_filterCombobox->addItem(i18n("Jupyter files"), QLatin1String("*.ipynb"));
    m_filterCombobox->addItem(i18n("All supported files"), QLatin1String("*.cws *.ipynb"));
    m_filterCombobox->addItem(i18n("All files"), QLatin1String("*"));
    connect(m_filterCombobox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &FileBrowserPanelPlugin::handleFilterChanging);
    m_model->setNameFilters({QLatin1String("*.cws")});
    m_model->setNameFilterDisables(false);

    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    horizontalLayout->setDirection(QBoxLayout::LeftToRight);
    horizontalLayout->addWidget(dirPreviousButton);
    horizontalLayout->addWidget(dirUpButton);
    horizontalLayout->addWidget(homeButton);
    horizontalLayout->addWidget(dirNextButton);
    horizontalLayout->addWidget(m_pathEdit);
    horizontalLayout->addWidget(m_filterCombobox);
    horizontalLayout->setMargin(0);

    buttonContainer->setLayout(horizontalLayout);
    buttonContainer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(buttonContainer);
    layout->addWidget(m_treeview);

    m_mainWidget->setLayout(layout);
}

void FileBrowserPanelPlugin::moveFileBrowserRoot(const QString& path)
{
    for (int i = 0; i < historyBackCount; i++)
        m_rootDirsHistory.pop_back();
    historyBackCount = 0;

    m_rootDirsHistory.push_back(path);
    setRootPath(path);
}

void FileBrowserPanelPlugin::setRootPath(const QString& path)
{
    m_model->setRootPath(path);
    m_treeview->setRootIndex(m_model->index(path));
    m_pathEdit->setText(path);
}

void FileBrowserPanelPlugin::dirUpButtonHandle()
{
    QDir dir(m_model->rootPath());
    if (dir.cdUp())
        moveFileBrowserRoot(dir.absolutePath());
}

void FileBrowserPanelPlugin::homeButtonHandle()
{
    moveFileBrowserRoot(QDir::homePath());
}

void FileBrowserPanelPlugin::dirNextButtonHandle()
{
    if (historyBackCount <= 0)
        return;
    historyBackCount -= 1;

    const QString& newPath = m_rootDirsHistory[m_rootDirsHistory.size() - 1 - historyBackCount];

    setRootPath(newPath);
}

void FileBrowserPanelPlugin::dirPreviousButtonHandle()
{
    if (historyBackCount >= m_rootDirsHistory.size() - 1)
        return;
    historyBackCount += 1;

    const QString& newPath = m_rootDirsHistory[m_rootDirsHistory.size() - 1 - historyBackCount];
    setRootPath(newPath);
}

void FileBrowserPanelPlugin::setNewRootPath()
{
    QString path = m_pathEdit->text();
    QFileInfo info(path);
    if (info.isDir())
        moveFileBrowserRoot(path);
}

void FileBrowserPanelPlugin::handleFilterChanging(int index)
{
    if (m_model)
        m_model->setNameFilters(m_filterCombobox->itemData(index).toString().split(QLatin1Char(' ')));
}

K_PLUGIN_FACTORY_WITH_JSON(filebrowserpanelplugin, "filebrowserpanelplugin.json", registerPlugin<FileBrowserPanelPlugin>();)
#include "filebrowserpanelplugin.moc"
