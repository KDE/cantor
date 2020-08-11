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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
    Copyright (C) 2019 Alexander Semke <alexander.semke@web.de>
    Copyright (C) 2020 Shubham <aryan100jangid@gmail.com>
 */

#include "maximabackend.h"
#include "maximaextensions.h"
#include "maximasession.h"
#include "settings.h"

#include <QDialog>
#include <QHelpEngineCore>
#include <QPointer>
#include <QToolButton>
#include <QTreeWidgetItem>

#include <KMessageBox>

enum Column
{
    NameColumn,
    PathColumn,
    IconColumn,
    GhnsColumn,
    ConfigColumn
};

MaximaBackend::MaximaBackend( QObject* parent,const QList<QVariant> args ) : Cantor::Backend( parent,args )
{
    //initialize the supported extensions
    new MaximaHistoryExtension(this);
    new MaximaScriptExtension(this);
    new MaximaCASExtension(this);
    new MaximaCalculusExtension(this);
    new MaximaLinearAlgebraExtension(this);
    new MaximaPlotExtension(this);
    new MaximaVariableManagementExtension(this);
}

MaximaBackend::~MaximaBackend()
{
    qDebug()<<"Destroying MaximaBackend";
}

QString MaximaBackend::id() const
{
    return QLatin1String("maxima");
}

QString MaximaBackend::version() const
{
    return QLatin1String("5.41, 5.42");
}

Cantor::Session* MaximaBackend::createSession()
{
    qDebug()<<"Spawning a new Maxima session";

    return new MaximaSession(this);
}

Cantor::Backend::Capabilities MaximaBackend::capabilities() const
{
    Cantor::Backend::Capabilities cap =
        Cantor::Backend::LaTexOutput |
        Cantor::Backend::InteractiveMode|
        Cantor::Backend::SyntaxHighlighting|
        Cantor::Backend::Completion |
        Cantor::Backend::SyntaxHelp;
    if(MaximaSettings::self()->variableManagement())
        cap |= Cantor::Backend::VariableManagement;

    return cap;
}

bool MaximaBackend::requirementsFullfilled(QString* const reason) const
{
    const QString& path = MaximaSettings::self()->path().toLocalFile();
    return Cantor::Backend::checkExecutable(QLatin1String("Maxima"), path, reason);
}

QUrl MaximaBackend::helpUrl() const
{
    const QUrl& localDoc = MaximaSettings::self()->localDoc();
    if (!localDoc.isEmpty())
        return localDoc;
    else
        return QUrl(i18nc("the url to the documentation of Maxima, please check if there is a translated version and use the correct url",
            "http://maxima.sourceforge.net/docs/manual/en/maxima.html"));
}

QWidget* MaximaBackend::settingsWidget(QWidget* parent) const
{
    QWidget* widget = new QWidget(parent);
    m_settings = new Ui::MaximaSettingsBase;
    m_settings->setupUi(widget);

    // KNewStuff button (Get New Documentation)
    /*auto* knsButton = new KNS3::Button(i18nc("@action:button Allow user to get some API documentation with GHNS", "Get New Documentation"), QStringLiteral("cantor_maxima.knsrc"), m_settings->groupBox2);
    m_settings->horizontalLayout_2->insertWidget(1, knsButton);
    connect(knsButton, &KNS3::Button::dialogFinished, this, &MaximaBackend::knsUpdate);*/

    connect(m_settings->add, &QPushButton::clicked, this, &MaximaBackend::add);
    return widget;
}

KConfigSkeleton* MaximaBackend::config() const
{
    return MaximaSettings::self();
}

QString MaximaBackend::description() const
{
    return i18n("<b>Maxima</b> is a system for the manipulation of symbolic and numerical expressions, "\
                "including differentiation, integration, Taylor series, Laplace transforms, "\
                "ordinary differential equations, systems of linear equations, polynomials, and sets, "\
                "lists, vectors, matrices, and tensors. Maxima yields high precision numeric results "\
                "by using exact fractions, arbitrary precision integers, and variable precision "\
                "floating point numbers. Maxima can plot functions and data in two and three dimensions.");
}

void MaximaBackend::add()
{
    QPointer<QtHelpConfigEditDialog> dialog = new QtHelpConfigEditDialog(nullptr);

    if (dialog->exec())
    {
        QTreeWidgetItem* item = addTableItem(dialog->qchIcon->icon(), dialog->qchName->text(), dialog->qchRequester->text(), QStringLiteral("0"));
        m_settings->qchTable->setCurrentItem(item);
    }

    delete dialog;
}

void MaximaBackend::remove(QTreeWidgetItem*)
{

}

void MaximaBackend::modify(QTreeWidgetItem* item)
{

}

void MaximaBackend::knsUpdate(const KNS3::Entry::List& list)
{
Q_UNUSED(list)
}

bool MaximaBackend::checkNamespace(const QString& filename, QTreeWidgetItem* modifiedItem)
{
    QString qtHelpNamespace = QHelpEngineCore::namespaceName(filename);
    if (qtHelpNamespace.isEmpty()) {
        // Open error message (not valid Qt Compressed Help file)
        KMessageBox::error(nullptr, i18n("Qt Compressed Help file is not valid."));
        return false;
    }
    // verify if it's the namespace it's not already in the list
    for(int i=0; i < m_settings->qchTable->topLevelItemCount(); i++) {
        const QTreeWidgetItem* item = m_settings->qchTable->topLevelItem(i);
        if (item != modifiedItem){
            if (qtHelpNamespace == QHelpEngineCore::namespaceName(item->text(PathColumn))) {
                // Open error message, documentation already imported
                KMessageBox::error(nullptr, i18n("Documentation already imported"));
                return false;
            }
        }
    }
    return true;
}

QTreeWidgetItem* MaximaBackend::addTableItem(const QString &icon, const QString &name, const QString &path, const QString &ghnsStatus)
{
    auto *item = new QTreeWidgetItem(m_settings->qchTable);
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
    m_settings->qchTable->setItemWidget(item, ConfigColumn, ctrlWidget);

    return item;
}

K_PLUGIN_FACTORY_WITH_JSON(maximabackend, "maximabackend.json", registerPlugin<MaximaBackend>();)
#include "maximabackend.moc"
