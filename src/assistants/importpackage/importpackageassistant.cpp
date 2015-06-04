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
    Copyright (C) 2013 Filipe Saraiva <filipe@kde.org>
 */

#include "importpackageassistant.h"

#include <QAction>

#include <QDialog>
#include <QPushButton>
#include <KActionCollection>
#include <KConfigGroup>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"
#include "ui_importpackagedlg.h"

ImportPackageAssistant::ImportPackageAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

ImportPackageAssistant::~ImportPackageAssistant()
{

}

void ImportPackageAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_import_package_assistant.rc"));

    QAction* importpackage = new QAction(i18n("Import Package"), actionCollection());
    actionCollection()->addAction(QLatin1String("importpackage_assistant"), importpackage);
    connect(importpackage, &QAction::triggered, this, &ImportPackageAssistant::requested);
}

QStringList ImportPackageAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);

    QWidget* widget=new QWidget(dlg);

    Ui::ImportPackageAssistantBase base;
    base.setupUi(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(base.buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    QStringList result;
    if( dlg->exec())
    {
        const QString& m = base.package->text();

        Cantor::PackagingExtension* ext =
            dynamic_cast<Cantor::PackagingExtension*>(backend()->extension(QLatin1String("PackagingExtension")));
        result << ext->importPackage(m);
    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(importpackageassistant, ImportPackageAssistant)
#include "importpackageassistant.moc"
