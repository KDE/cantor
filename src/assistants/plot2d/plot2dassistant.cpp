/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "plot2dassistant.h"

#include <QAction>
#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include <KConfigGroup>
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
    QAction* plot2d=new QAction(i18n("Plot 2D"), actionCollection());
    actionCollection()->addAction(QLatin1String("plot2d_assistant"), plot2d);
    connect(plot2d, &QAction::triggered, this, &Plot2dAssistant::requested);
}

QStringList Plot2dAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::Plot2dAssistantBase base;
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
        const QString expression=base.expression->text();
        const QString variable=base.variable->text();
        const QString min=base.min->text();
        const QString max=base.max->text();

        Cantor::PlotExtension* ext=
            dynamic_cast<Cantor::PlotExtension*>(backend()->extension(QLatin1String("PlotExtension")));

        result<<ext->plotFunction2d(expression, variable, min, max);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(plot2dassistant, "plot2dassistant.json", registerPlugin<Plot2dAssistant>();)
#include "plot2dassistant.moc"
