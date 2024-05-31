/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#include "importpackageassistant.h"

#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"
#include "ui_importpackagedlg.h"

ImportPackageAssistant::ImportPackageAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void ImportPackageAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_import_package_assistant.rc"));

    auto* importpackage = new QAction(i18n("Import Package"), actionCollection());
    actionCollection()->addAction(QLatin1String("importpackage_assistant"), importpackage);
    connect(importpackage, &QAction::triggered, this, &ImportPackageAssistant::requested);
}

QStringList ImportPackageAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg = new QDialog(parent);
    auto* widget = new QWidget(dlg);
    Ui::ImportPackageAssistantBase base;
    base.setupUi(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(base.buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    QStringList result;
    if( dlg->exec())
    {
        auto* ext = dynamic_cast<Cantor::PackagingExtension*>(backend()->extension(QLatin1String("PackagingExtension")));
        if (ext)
            result << ext->importPackage(base.package->text());
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(importpackageassistant, "importpackageassistant.json", registerPlugin<ImportPackageAssistant>();)
#include "importpackageassistant.moc"
