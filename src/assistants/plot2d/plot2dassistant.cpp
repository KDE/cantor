/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "plot2dassistant.h"

#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include "ui_plot2ddlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

Plot2dAssistant::Plot2dAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void Plot2dAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_plot2d_assistant.rc"));
    auto* plot2d = new QAction(i18n("Plot 2D"), actionCollection());
    actionCollection()->addAction(QLatin1String("plot2d_assistant"), plot2d);
    connect(plot2d, &QAction::triggered, this, &Plot2dAssistant::requested);
}

QStringList Plot2dAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg = new QDialog(parent);
    auto* widget = new QWidget(dlg);
    Ui::Plot2dAssistantBase base;
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
        auto* ext = dynamic_cast<Cantor::PlotExtension*>(backend()->extension(QLatin1String("PlotExtension")));
        if (ext)
        {
            const auto& expression = base.expression->text();
            const auto& variable = base.variable->text();
            const auto& min = base.min->text();
            const auto& max = base.max->text();
            result<<ext->plotFunction2d(expression, variable, min, max);
        }
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(plot2dassistant, "plot2dassistant.json", registerPlugin<Plot2dAssistant>();)
#include "plot2dassistant.moc"
