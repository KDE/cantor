/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "differentiateassistant.h"

#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
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
    auto* differentiate = new QAction(i18n("Differentiate"), actionCollection());
    differentiate->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("differentiate_assistant"), differentiate);
    connect(differentiate, &QAction::triggered, this, &DifferentiateAssistant::requested);
}

QStringList DifferentiateAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    auto* widget = new QWidget(dlg);
    Ui::DifferentiateAssistantBase base;
    base.setupUi(widget);
    auto* mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(base.buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    QStringList result;
    if( dlg->exec())
    {
        auto* ext = dynamic_cast<Cantor::CalculusExtension*>(backend()->extension(QLatin1String("CalculusExtension")));
        if (ext)
        {
            const auto& expression = base.expression->text();
            const auto& variable = base.variable->text();
            const int times = base.times->value();
            result << ext->differentiate(expression, variable, times);
        }
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(differentiateassistant, "differentiateassistant.json", registerPlugin<DifferentiateAssistant>();)
#include "differentiateassistant.moc"
