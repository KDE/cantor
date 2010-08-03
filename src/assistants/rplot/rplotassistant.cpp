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

#include "rplotassistant.h"

#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include "ui_rplotdialog.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

RPlotAssistant::RPlotAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

RPlotAssistant::~RPlotAssistant()
{

}

void RPlotAssistant::initActions()
{
    setXMLFile("cantor_rplot_assistant.rc");
    KAction* rplot=new KAction(i18n("R-Plot STUB"), actionCollection());
    actionCollection()->addAction("rplot_assistant", rplot);
    connect(rplot, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList RPlotAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::RPlotAssistantBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

    QStringList result;
    if( dlg->exec())
    {
       Cantor::PlotExtension* ptr= dynamic_cast<Cantor::PlotExtension*>(backend()->extension("PlotExtension"));
    /*   result<<ptr->RPlot(base.expression->text(),base.captionEdit->text(),
            base.XGroup->isChecked()? base.XLabEdit->text() : "",
            base.YGroup->isChecked()? base.YLabEdit->text() : "",
            base.XGroup->isChecked() && base.XRangeGroup->isChecked(), base.XMinEdit->value(), base.XMaxEdit->value(),
            base.YGroup->isChecked() && base.YRangeGroup->isChecked(), base.XMinEdit->value(), base.XMaxEdit->value()); */
    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(rplotassistant, RPlotAssistant)
