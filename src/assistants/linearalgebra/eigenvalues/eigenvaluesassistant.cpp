/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "eigenvaluesassistant.h"

#include <QAction>
#include <QDialog>
#include <QPushButton>
#include <QStyle>

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
