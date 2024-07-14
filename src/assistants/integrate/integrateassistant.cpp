/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "integrateassistant.h"

#include <QIcon>
#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include "ui_integratedlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

IntegrateAssistant::IntegrateAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void IntegrateAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_integrate_assistant.rc"));
    QAction* integrate=new QAction(i18n("Integrate"), actionCollection());
    integrate->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("integrate_assistant"), integrate);
    connect(integrate, &QAction::triggered, this, &IntegrateAssistant::requested);
}

QStringList IntegrateAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg = new QDialog(parent);
    auto* widget = new QWidget(dlg);
    Ui::IntegrateAssistantBase base;
    base.setupUi(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
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
            if (base.isDefinite->isChecked())
            {
                const auto& lower = base.lowerLimit->text();
                const auto& upper = base.upperLimit->text();
                result << ext->integrate(expression, variable, lower, upper);
            }
            else
                result << ext->integrate(expression, variable);
        }
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(integrateassistant, "integrateassistant.json", registerPlugin<IntegrateAssistant>();)
#include "integrateassistant.moc"
