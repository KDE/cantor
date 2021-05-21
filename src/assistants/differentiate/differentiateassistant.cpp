/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "differentiateassistant.h"

#include <QAction>
#include <QIcon>
#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include <KConfigGroup>
#include "ui_differentiatedlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

DifferentiateAssistant::DifferentiateAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void DifferentiateAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_differentiate_assistant.rc"));
    QAction* differentiate=new QAction(i18n("Differentiate"), actionCollection());
    differentiate->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("differentiate_assistant"), differentiate);
    connect(differentiate, &QAction::triggered, this, &DifferentiateAssistant::requested);
}

QStringList DifferentiateAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget* widget=new QWidget(dlg);
    Ui::DifferentiateAssistantBase base;
    base.setupUi(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(base.buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

    QStringList result;
    if( dlg->exec())
    {
        QString expression=base.expression->text();
        QString variable=base.variable->text();
        int times=base.times->value();

        Cantor::CalculusExtension* ext=
            dynamic_cast<Cantor::CalculusExtension*>(backend()->extension(QLatin1String("CalculusExtension")));

        result<<ext->differentiate(expression, variable, times);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(differentiateassistant, "differentiateassistant.json", registerPlugin<DifferentiateAssistant>();)
#include "differentiateassistant.moc"
