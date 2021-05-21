/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "advancedplotassistant.h"

#include <QAction>
#include <QDialog>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KConfigGroup>
#include "ui_advancedplotdialog.h"
#include "ui_directivecontainer.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

AdvancedPlotAssistant::AdvancedPlotAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void AdvancedPlotAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_advancedplot_assistant.rc"));
    QAction* advplot=new QAction(i18n("Advanced Plotting"), actionCollection());
    actionCollection()->addAction(QLatin1String("advancedplot_assistant"), advplot);
    connect(advplot, &QAction::triggered, this, &AdvancedPlotAssistant::requested);
}

QStringList AdvancedPlotAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::AdvancedPlotAssistantBase base;
    base.setupUi(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(base.buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

    //Casting the extension to correct type and checking it
    Cantor::AdvancedPlotExtension * plotter=dynamic_cast<Cantor::AdvancedPlotExtension*>
        (backend()->extension(QLatin1String("AdvancedPlotExtension")));
    if (plotter==nullptr)
    {
        qDebug()<<"Advanced plotting extension is messed up, that's a bug.";
	delete dlg;
        return QStringList();
    }

    //Filling up the form accordingly
    Cantor::AdvancedPlotExtension::AcceptorBase *acceptor=dynamic_cast<Cantor::AdvancedPlotExtension::AcceptorBase*>(plotter);
    QVector<Cantor::AdvancedPlotExtension::DirectiveProducer *> controls;
    if (acceptor!=nullptr)
    {
        foreach (const Cantor::AdvancedPlotExtension::AcceptorBase::widgetProc& wProc, acceptor->widgets())
        {
            QGroupBox *container=new QGroupBox(nullptr);
            Cantor::AdvancedPlotExtension::DirectiveProducer *cargo=wProc(nullptr);
            Ui::directiveContainer uicont;
            uicont.setupUi(container);
            QVBoxLayout *layout=new QVBoxLayout;
            layout->addWidget(cargo);
            container->setLayout(layout);
            base.directivesTabs->addTab(container,cargo->windowTitle());
            controls.push_back(cargo);
        }
    }

    QStringList result;
    if( dlg->exec())
    {
        QVector<Cantor::AdvancedPlotExtension::PlotDirective*> list;
        //FIXME lots of dynamic casts :(
        for (int i=0;i<base.directivesTabs->count();i++)
        {
            QGroupBox *group=dynamic_cast<QGroupBox*>(base.directivesTabs->widget(i));
            if (group!=nullptr)
                if (group->isChecked())
                    list.push_back(controls[i]->produceDirective());
        }
        result<<plotter->plotFunction2d(base.expressionEdit->text(),list);
        qDeleteAll(list);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(advancedplotassistant, "advancedplotassistant.json", registerPlugin<AdvancedPlotAssistant>();)
#include "advancedplotassistant.moc"
