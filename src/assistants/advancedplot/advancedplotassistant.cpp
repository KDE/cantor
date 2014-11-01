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

#include "advancedplotassistant.h"

#include <QVBoxLayout>
#include <KDialog>
#include <KAction>
#include <KActionCollection>
#include "ui_advancedplotdialog.h"
#include "ui_directivecontainer.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

AdvancedPlotAssistant::AdvancedPlotAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

AdvancedPlotAssistant::~AdvancedPlotAssistant()
{

}

void AdvancedPlotAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_advancedplot_assistant.rc"));
    KAction* advplot=new KAction(i18n("Advanced Plotting"), actionCollection());
    actionCollection()->addAction(QLatin1String("advancedplot_assistant"), advplot);
    connect(advplot, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList AdvancedPlotAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::AdvancedPlotAssistantBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

    //Casting the extension to correct type and checking it
    Cantor::AdvancedPlotExtension * plotter=dynamic_cast<Cantor::AdvancedPlotExtension*>
        (backend()->extension(QLatin1String("AdvancedPlotExtension")));
    if (plotter==NULL)
    {
        qDebug()<<"Advanced plotting extension is messed up, that's a bug.";
	delete dlg;
        return QStringList();
    }

    //Filling up the form accordingly
    Cantor::AdvancedPlotExtension::AcceptorBase *acceptor=dynamic_cast<Cantor::AdvancedPlotExtension::AcceptorBase*>(plotter);
    QVector<Cantor::AdvancedPlotExtension::DirectiveProducer *> controls;
    if (acceptor!=NULL)
    {
        foreach (const Cantor::AdvancedPlotExtension::AcceptorBase::widgetProc& wProc, acceptor->widgets())
        {
            QGroupBox *container=new QGroupBox(NULL);
            Cantor::AdvancedPlotExtension::DirectiveProducer *cargo=wProc(NULL);
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
            if (group!=NULL)
                if (group->isChecked())
                    list.push_back(controls[i]->produceDirective());
        }
        result<<plotter->plotFunction2d(base.expressionEdit->text(),list);
        qDeleteAll(list);
    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(advancedplotassistant, AdvancedPlotAssistant)
#include "advancedplotassistant.moc"
