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

#include "integrateassistant.h"

#include <KDialog>
#include <KAction>
#include <KActionCollection>
#include <KIcon>
#include "ui_integratedlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

IntegrateAssistant::IntegrateAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

IntegrateAssistant::~IntegrateAssistant()
{

}

void IntegrateAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_integrate_assistant.rc"));
    KAction* integrate=new KAction(i18n("Integrate"), actionCollection());
    integrate->setIcon(KIcon(icon()));
    actionCollection()->addAction(QLatin1String("integrate_assistant"), integrate);
    connect(integrate, &KAction::triggered, this, &IntegrateAssistant::requested);
}

QStringList IntegrateAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);
    QWidget* widget=new QWidget(dlg);
    Ui::IntegrateAssistantBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

    QStringList result;
    if( dlg->exec())
    {
        QString expression=base.expression->text();
        QString variable=base.variable->text();

        Cantor::CalculusExtension* ext=
            dynamic_cast<Cantor::CalculusExtension*>(backend()->extension(QLatin1String("CalculusExtension")));
        if (base.isDefinite->isChecked())
        {
            QString lower=base.lowerLimit->text();
            QString upper=base.upperLimit->text();

            result<<ext->integrate(expression, variable, lower, upper);
        }else
        {
            result<<ext->integrate(expression, variable);
        }
    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(integrateassistant, IntegrateAssistant)
#include "integrateassistant.moc"
