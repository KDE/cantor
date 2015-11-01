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
 */

#include "eigenvaluesassistant.h"

#include <QAction>

#include <QDialog>
#include <QPushButton>
#include <KActionCollection>
#include <KConfigGroup>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"
#include "ui_eigenvaluesdlg.h"

EigenValuesAssistant::EigenValuesAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

EigenValuesAssistant::~EigenValuesAssistant()
{

}

void EigenValuesAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_eigenvalues_assistant.rc"));
    QAction* eigenvalues=new QAction(i18n("Compute Eigenvalues"), actionCollection());
    actionCollection()->addAction(QLatin1String("eigenvalues_assistant"), eigenvalues);
    connect(eigenvalues, &QAction::triggered, this, &EigenValuesAssistant::requested);
}

QStringList EigenValuesAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget* widget=new QWidget(dlg);
    Ui::EigenValuesAssistantBase base;
    base.setupUi(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(base.buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

    Cantor::HistoryExtension* hist=
        dynamic_cast<Cantor::HistoryExtension*>(backend()->extension(QLatin1String("HistoryExtension")));
    base.matrix->setText(hist->lastResult());

    QStringList result;
    if( dlg->exec())
    {
        const QString& m=base.matrix->text();
        Cantor::LinearAlgebraExtension* ext=
            dynamic_cast<Cantor::LinearAlgebraExtension*>(backend()->extension(QLatin1String("LinearAlgebraExtension")));
        result<<ext->eigenValues(m);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(eigenvaluesassistant, "eigenvaluesassistant.json", registerPlugin<EigenValuesAssistant>();)
#include "eigenvaluesassistant.moc"
