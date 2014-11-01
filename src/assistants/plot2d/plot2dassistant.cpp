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

#include "plot2dassistant.h"

#include <KDialog>
#include <KAction>
#include <KActionCollection>
#include "ui_plot2ddlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

Plot2dAssistant::Plot2dAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

Plot2dAssistant::~Plot2dAssistant()
{

}

void Plot2dAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_plot2d_assistant.rc"));
    KAction* plot2d=new KAction(i18n("Plot 2D"), actionCollection());
    //plot2d->setIcon(KIcon(icon()));
    actionCollection()->addAction(QLatin1String("plot2d_assistant"), plot2d);
    connect(plot2d, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList Plot2dAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::Plot2dAssistantBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

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

K_EXPORT_CANTOR_PLUGIN(plot2dassistant, Plot2dAssistant)
#include "plot2dassistant.moc"
