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

#include "plot3dassistant.h"

#include <QAction>
#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include <KConfigGroup>
#include "ui_plot3ddlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

Plot3dAssistant::Plot3dAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

Plot3dAssistant::~Plot3dAssistant()
{

}

void Plot3dAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_plot3d_assistant.rc"));
    QAction* plot3d=new QAction(i18n("Plot 3D"), actionCollection());
    actionCollection()->addAction(QLatin1String("plot3d_assistant"), plot3d);
    connect(plot3d, &QAction::triggered, this, &Plot3dAssistant::requested);
}

QStringList Plot3dAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::Plot3dAssistantBase base;
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
        Cantor::PlotExtension::VariableParameter v1;
        Cantor::PlotExtension::Interval i1;
        v1.first=base.variable1->text();
        i1.first=base.min1->text();
        i1.second=base.max1->text();
        v1.second=i1;

        Cantor::PlotExtension::VariableParameter v2;
        Cantor::PlotExtension::Interval i2;
        v2.first=base.variable2->text();
        i2.first=base.min2->text();
        i2.second=base.max2->text();
        v2.second=i2;

        Cantor::PlotExtension* ext=
            dynamic_cast<Cantor::PlotExtension*>(backend()->extension(QLatin1String("PlotExtension")));

        result<<ext->plotFunction3d(expression, v1, v2);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(plot3dassistant, "plot3dassistant.json", registerPlugin<Plot3dAssistant>();)
#include "plot3dassistant.moc"
