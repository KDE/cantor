/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
    SPDX-FileCopyrightText: 2020-2021 Alexander Semke <alexander.semke@web.de>
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

        if (modifiedItem)
            setWindowTitle(i18nc("@title:window", "Modify Entry"));
        else
            setWindowTitle(i18nc("@title:window", "Add New Entry"));

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

QtHelpConfig::QtHelpConfig(const QString& backend) : QWidget(), m_backend(backend)
{

    Ui::QtHelpConfigUI* ui = new Ui::QtHelpConfigUI;
    ui->setupUi(this);
    ui->addButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    connect(ui->addButton, &QPushButton::clicked, this, &QtHelpConfig::add);

    m_treeWidget = ui->qchTable;

    // Table
    m_treeWidget->setColumnHidden(IconColumn, true);
    m_treeWidget->setColumnHidden(GhnsColumn, true);
    m_treeWidget->model()->setHeaderData(ConfigColumn, Qt::Horizontal, QVariant());
    m_treeWidget->header()->setSectionsMovable(false);
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(PathColumn, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(ConfigColumn, QHeaderView::Fixed);

    // Add GHNS button // shift this code to backend specific
    auto* knsButton = new KNS3::Button(i18nc("@action:button Allow user to get some API documentation with GHNS", "Get New Documentation"),
                                       QStringLiteral("cantor-qthelp.knsrc"),
                                       this);
    ui->tableCtrlLayout->insertWidget(1, knsButton);
    connect(knsButton, &KNS3::Button::dialogFinished, this, &QtHelpConfig::knsUpdate);

    connect(this, &QtHelpConfig::settingsChanged, this, &QtHelpConfig::saveSettings);

    // load settings for Install Additional Help Files widget
    loadSettings();
}

QtHelpConfig::~QtHelpConfig() = default;

void QtHelpConfig::add()
{
    QPointer<QtHelpConfigEditDialog> dialog = new QtHelpConfigEditDialog(nullptr, this);
    if (dialog->exec()) {
        auto* item = addTableItem(dialog->qchIcon->icon(),
                                  dialog->qchName->text(),
                                  dialog->qchRequester->text(),
                                  QStringLiteral("0"));
        m_treeWidget->setCurrentItem(item);
        emit settingsChanged();
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
        emit settingsChanged();
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
    for(int i=0; i < m_treeWidget->topLevelItemCount(); i++) {
        const QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
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
    emit settingsChanged();
}

void QtHelpConfig::knsUpdate(const KNS3::Entry::List& list)
{
    if (list.isEmpty())
        return;

    for (const auto& e : list) {
        if(e.status() == KNS3::Entry::Installed) {
            // For zipped/tarred QCH files KNewStuff also adds the directory as installed file, first file entry is assumed to be QCH file though
            if (e.installedFiles().size() >= 1) {
                const auto& filename = e.installedFiles().at(0);
                if(checkNamespace(filename, nullptr)){
                    auto* item = addTableItem(QStringLiteral("documentation"), e.name(), filename, QStringLiteral("1"));
                    m_treeWidget->setCurrentItem(item);
                } else {
                    qDebug() << "namespace error";
                }
            }
        } else if(e.status() ==  KNS3::Entry::Deleted) {
            // cmp. note above for installed files
            if (e.uninstalledFiles().size() >= 1) {
                for(int i=0; i < m_treeWidget->topLevelItemCount(); i++) {
                    const auto* item = m_treeWidget->topLevelItem(i);
                    if (e.uninstalledFiles().at(0) == item->text(PathColumn)) {
                        delete item;
                        break;
                    }
                }
            }
        }
    }
    emit settingsChanged();
}

QTreeWidgetItem* QtHelpConfig::addTableItem(const QString& icon, const QString& name,
                                             const QString& path, const QString& ghnsStatus)
{
    auto* item = new QTreeWidgetItem(m_treeWidget);
    item->setIcon(NameColumn, QIcon::fromTheme(icon));
    item->setText(NameColumn, name);
    item->setToolTip(NameColumn, name);
    item->setText(PathColumn, path);
    item->setToolTip(PathColumn, path);
    item->setText(IconColumn, icon);
    item->setText(GhnsColumn, ghnsStatus);

    auto* ctrlWidget = new QWidget(item->treeWidget());
    ctrlWidget->setLayout(new QHBoxLayout(ctrlWidget));

    auto* modifyBtn = new QToolButton(item->treeWidget());
    modifyBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    modifyBtn->setToolTip(i18nc("@info:tooltip", "Modify"));
    connect(modifyBtn, &QPushButton::clicked, this, [=](){ modify(item); });

    auto *removeBtn = new QToolButton(item->treeWidget());
    removeBtn->setIcon(QIcon::fromTheme(QStringLiteral("entry-delete")));
    removeBtn->setToolTip(i18nc("@info:tooltip", "Delete"));

    if (item->text(GhnsColumn) != QLatin1String("0"))
    {
        // KNS3 currently does not provide API to uninstall entries
        // just removing the files results in wrong installed states in the KNS3 dialog
        // TODO: add API to KNS to remove files without UI interaction
        removeBtn->setEnabled(false);
        removeBtn->setToolTip(i18nc("@info:tooltip", "Please uninstall this via GHNS."));
    } else
        connect(removeBtn, &QPushButton::clicked, this, [=](){ remove(item); });

    ctrlWidget->layout()->addWidget(modifyBtn);
    ctrlWidget->layout()->addWidget(removeBtn);

    m_treeWidget->setItemWidget(item, ConfigColumn, ctrlWidget);

    return item;
}

void QtHelpConfig::loadSettings()
{
    // load settings for current backend and then update the QTreeWidget
    const auto& group = KSharedConfig::openConfig()->group(m_backend);

    const auto& nameList = group.readEntry(QLatin1String("Names"), QStringList());
    const auto& pathList = group.readEntry(QLatin1String("Paths"), QStringList());
    const auto& iconList = group.readEntry(QLatin1String("Icons"), QStringList());
    const auto& ghnsList = group.readEntry(QLatin1String("Ghns"), QStringList());

    // iterate through Name Location pairs and update the QTreeWidget
    for(int i = 0; i < nameList.size(); i++)
    {
        QTreeWidgetItem* item = addTableItem(iconList.at(i), nameList.at(i), pathList.at(i), ghnsList.at(i));
        m_treeWidget->setCurrentItem(item);
    }
}

void QtHelpConfig::saveSettings()
{
    // create seperate group for seperate backends
    KConfigGroup group = KSharedConfig::openConfig()->group(m_backend);

    QStringList nameList;
    QStringList pathList;
    QStringList iconList;
    QStringList ghnsList;

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
    {
        const auto* item = m_treeWidget->topLevelItem(i);
        nameList << item->text(0);
        pathList << item->text(1);
        iconList << item->text(2);
        ghnsList << item->text(3);
    }

    group.writeEntry(QLatin1String("Names"), nameList);
    group.writeEntry(QLatin1String("Paths"), pathList);
    group.writeEntry(QLatin1String("Icons"), iconList);
    group.writeEntry(QLatin1String("Ghns"), ghnsList);
}

#include "qthelpconfig.moc"
