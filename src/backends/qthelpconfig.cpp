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

#include <QDebug>
#include <QHeaderView>
#include <QHelpEngineCore>
#include <QPointer>
#include <QToolButton>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNS3/Button>
#include <KSharedConfig>

#include "qthelpconfig.h"

#include "ui_qthelpconfigeditdialog.h"
#include "ui_qthelpconfig.h"

enum Column
{
    NameColumn,
    PathColumn,
    IconColumn,
    GhnsColumn,
    ConfigColumn
};

class QtHelpConfigEditDialog : public QDialog, public Ui_QtHelpConfigEditDialog
{
    Q_OBJECT
public:
    explicit QtHelpConfigEditDialog(QTreeWidgetItem* modifiedItem, QtHelpConfig* parent = nullptr)
        : QDialog(parent)
        , m_modifiedItem(modifiedItem)
        , m_config(parent)
    {
        setupUi(this);

        if (modifiedItem) {
            setWindowTitle(i18nc("@title:window", "Modify Entry"));
        } else {
            setWindowTitle(i18nc("@title:window", "Add New Entry"));
        }
        qchIcon->setIcon(QStringLiteral("qtlogo"));
    }

    bool checkQtHelpFile();

    void accept() override;

private:
    QTreeWidgetItem* m_modifiedItem;
    QtHelpConfig* m_config;
};

bool QtHelpConfigEditDialog::checkQtHelpFile()
{
    //verify if the file is valid and if there is a name
    if(qchName->text().isEmpty()){
        KMessageBox::error(this, i18n("Name cannot be empty."));
        return false;
    }

    return m_config->checkNamespace(qchRequester->text(), m_modifiedItem);
}

void QtHelpConfigEditDialog::accept()
{
    if (!checkQtHelpFile())
        return;

    QDialog::accept();
}

QtHelpConfig::QtHelpConfig()
{
    m_configWidget = new Ui::QtHelpConfigUI;
    m_configWidget->setupUi(this);
    m_configWidget->addButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    connect(m_configWidget->addButton, &QPushButton::clicked, this, &QtHelpConfig::add);

    // Table
    m_configWidget->qchTable->setColumnHidden(IconColumn, true);
    m_configWidget->qchTable->setColumnHidden(GhnsColumn, true);
    m_configWidget->qchTable->model()->setHeaderData(ConfigColumn, Qt::Horizontal, QVariant());
    m_configWidget->qchTable->header()->setSectionsMovable(false);
    m_configWidget->qchTable->header()->setStretchLastSection(false);
    m_configWidget->qchTable->header()->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    m_configWidget->qchTable->header()->setSectionResizeMode(PathColumn, QHeaderView::Stretch);
    m_configWidget->qchTable->header()->setSectionResizeMode(ConfigColumn, QHeaderView::Fixed);

    // Add GHNS button // shift this code to backend specific
    auto* knsButton = new KNS3::Button(i18nc("@action:button Allow user to get some API documentation with GHNS", "Get New Documentation"), QStringLiteral("kdevelop-qthelp.knsrc"), m_configWidget->boxQchManage);
    m_configWidget->tableCtrlLayout->insertWidget(1, knsButton);
    connect(knsButton, &KNS3::Button::dialogFinished, this, &QtHelpConfig::knsUpdate);

    reset();
}

QtHelpConfig::~QtHelpConfig()
{
    delete m_configWidget;
}

void QtHelpConfig::apply()
{
    QStringList iconList, nameList, pathList, ghnsList;
    for (int i = 0; i < m_configWidget->qchTable->topLevelItemCount(); i++) {
        const QTreeWidgetItem* item = m_configWidget->qchTable->topLevelItem(i);
        nameList << item->text(0);
        pathList << item->text(1);
        iconList << item->text(2);
        ghnsList << item->text(3);
    }

    qtHelpWriteConfig(iconList, nameList, pathList, ghnsList);
    //static_cast<QtHelpPlugin*>(plugin())->readConfig();*/
}

void QtHelpConfig::reset()
{
    m_configWidget->qchTable->clear();

    QStringList iconList, nameList, pathList, ghnsList;
    qtHelpReadConfig(iconList, nameList, pathList, ghnsList);

    const int size = qMin(qMin(iconList.size(), nameList.size()), pathList.size());
    for(int i = 0; i < size; ++i) {
        QString ghnsStatus = ghnsList.size()>i ? ghnsList.at(i) : QStringLiteral("0");
        addTableItem(iconList.at(i), nameList.at(i), pathList.at(i), ghnsStatus);
    }
}

void QtHelpConfig::defaults()
{
    if(m_configWidget->qchTable->topLevelItemCount() > 0) {
        m_configWidget->qchTable->clear();
    }
}

void QtHelpConfig::add()
{
    QPointer<QtHelpConfigEditDialog> dialog = new QtHelpConfigEditDialog(nullptr, this);
    if (dialog->exec()) {
        QTreeWidgetItem* item = addTableItem(dialog->qchIcon->icon(), dialog->qchName->text(), dialog->qchRequester->text(), QStringLiteral("0"));
        m_configWidget->qchTable->setCurrentItem(item);
    }
    delete dialog;
}

void QtHelpConfig::modify(QTreeWidgetItem* item)
{
    if (!item)
        return;

    QPointer<QtHelpConfigEditDialog> dialog = new QtHelpConfigEditDialog(item, this);

    if (item->text(GhnsColumn) != QLatin1String("0"))
    {
        dialog->qchRequester->setText(i18n("Documentation provided by GHNS"));
        dialog->qchRequester->setEnabled(false);
    }
    else
    {
        dialog->qchRequester->setText(item->text(PathColumn));
        dialog->qchRequester->setEnabled(true);
    }

    dialog->qchName->setText(item->text(NameColumn));
    dialog->qchIcon->setIcon(item->text(IconColumn));

    if (dialog->exec()) {
        item->setIcon(NameColumn, QIcon(dialog->qchIcon->icon()));
        item->setText(NameColumn, dialog->qchName->text());
        item->setText(IconColumn, dialog->qchIcon->icon());
        if(item->text(GhnsColumn) == QLatin1String("0")) {
            item->setText(PathColumn, dialog->qchRequester->text());
        }
    }
    delete dialog;
}

bool QtHelpConfig::checkNamespace(const QString& filename, QTreeWidgetItem* modifiedItem)
{
    QString qtHelpNamespace = QHelpEngineCore::namespaceName(filename);
    if (qtHelpNamespace.isEmpty()) {
        // Open error message (not valid Qt Compressed Help file)
        KMessageBox::error(this, i18n("Qt Compressed Help file is not valid."));
        return false;
    }
    // verify if it's the namespace it's not already in the list
    for(int i=0; i < m_configWidget->qchTable->topLevelItemCount(); i++) {
        const QTreeWidgetItem* item = m_configWidget->qchTable->topLevelItem(i);
        if (item != modifiedItem){
            if (qtHelpNamespace == QHelpEngineCore::namespaceName(item->text(PathColumn))) {
                // Open error message, documentation already imported
                KMessageBox::error(this, i18n("Documentation already imported"));
                return false;
            }
        }
    }
    return true;
}

void QtHelpConfig::remove(QTreeWidgetItem* item)
{
    if (!item)
        return;

    delete item;
}

void QtHelpConfig::knsUpdate(const KNS3::Entry::List& list)
{
    if (list.isEmpty())
        return;

    for (const KNS3::Entry& e : list) {
        if(e.status() == KNS3::Entry::Installed) {
            // For zipped/tarred QCH files KNewStuff also adds the directory as installed file, first file entry is assumed to be QCH file though
            if (e.installedFiles().size() >= 1) {
                QString filename = e.installedFiles().at(0);
                if(checkNamespace(filename, nullptr)){
                    QTreeWidgetItem* item = addTableItem(QStringLiteral("documentation"), e.name(), filename, QStringLiteral("1"));
                    m_configWidget->qchTable->setCurrentItem(item);
                } else {
                    qDebug() << "namespace error";
                }
            }
        } else if(e.status() ==  KNS3::Entry::Deleted) {
            // cmp. note above for installed files
            if (e.uninstalledFiles().size() >= 1) {
                for(int i=0; i < m_configWidget->qchTable->topLevelItemCount(); i++) {
                    QTreeWidgetItem* item = m_configWidget->qchTable->topLevelItem(i);
                    if (e.uninstalledFiles().at(0) == item->text(PathColumn)) {
                        delete item;
                        break;
                    }
                }
            }
        }
    }
}

QTreeWidgetItem * QtHelpConfig::addTableItem(const QString &icon, const QString &name,
                                             const QString &path, const QString &ghnsStatus)
{
    auto *item = new QTreeWidgetItem(m_configWidget->qchTable);
    item->setIcon(NameColumn, QIcon::fromTheme(icon));
    item->setText(NameColumn, name);
    item->setToolTip(NameColumn, name);
    item->setText(PathColumn, path);
    item->setToolTip(PathColumn, path);
    item->setText(IconColumn, icon);
    item->setText(GhnsColumn, ghnsStatus);

    auto* ctrlWidget = new QWidget(item->treeWidget());
    ctrlWidget->setLayout(new QHBoxLayout(ctrlWidget));

    auto *modifyBtn = new QToolButton(item->treeWidget());
    modifyBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    modifyBtn->setToolTip(i18nc("@info:tooltip", "Modify"));
    connect(modifyBtn, &QPushButton::clicked, this, [=](){
        modify(item);
    });
    auto *removeBtn = new QToolButton(item->treeWidget());
    removeBtn->setIcon(QIcon::fromTheme(QStringLiteral("entry-delete")));
    removeBtn->setToolTip(i18nc("@info:tooltip", "Delete"));
    if (item->text(GhnsColumn) != QLatin1String("0")) {
        // KNS3 currently does not provide API to uninstall entries
        // just removing the files results in wrong installed states in the KNS3 dialog
        // TODO: add API to KNS to remove files without UI interaction
        removeBtn->setEnabled(false);
        removeBtn->setToolTip(i18nc("@info:tooltip", "Please uninstall this via GHNS."));
    } else {
        connect(removeBtn, &QPushButton::clicked, this, [=](){
            remove(item);
        });
    }
    ctrlWidget->layout()->addWidget(modifyBtn);
    ctrlWidget->layout()->addWidget(removeBtn);
    m_configWidget->qchTable->setItemWidget(item, ConfigColumn, ctrlWidget);

    return item;
}

void QtHelpConfig::qtHelpReadConfig(QStringList& iconList, QStringList& nameList,
                      QStringList& pathList, QStringList& ghnsList)
{
    KConfigGroup cg(KSharedConfig::openConfig(), "QtHelp Documentation");
    iconList = cg.readEntry("iconList", QStringList());
    nameList = cg.readEntry("nameList", QStringList());
    pathList = cg.readEntry("pathList", QStringList());
    ghnsList = cg.readEntry("ghnsList", QStringList());
}

void QtHelpConfig::qtHelpWriteConfig(const QStringList& iconList, const QStringList& nameList,
                       const QStringList& pathList, const QStringList& ghnsList)
{
    KConfigGroup cg(KSharedConfig::openConfig(), "QtHelp Documentation");
    cg.writeEntry("iconList", iconList);
    cg.writeEntry("nameList", nameList);
    cg.writeEntry("pathList", pathList);
    cg.writeEntry("ghnsList", ghnsList);
}

#include "qthelpconfig.moc"
