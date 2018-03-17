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

#include <QAction>
#include <QIcon>
#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include <KConfigGroup>
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
    QAction* integrate=new QAction(i18n("Integrate"), actionCollection());
    integrate->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("integrate_assistant"), integrate);
    connect(integrate, &QAction::triggered, this, &IntegrateAssistant::requested);
}

QStringList IntegrateAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget* widget=new QWidget(dlg);
    Ui::IntegrateAssistantBase base;
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

K_PLUGIN_FACTORY_WITH_JSON(integrateassistant, "integrateassistant.json", registerPlugin<IntegrateAssistant>();)
#include "integrateassistant.moc"
