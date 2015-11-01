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

#include "differentiateassistant.h"

#include <QAction>
#include <QIcon>

#include <QDialog>
#include <QPushButton>
#include <KActionCollection>
#include <KConfigGroup>
#include "ui_differentiatedlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

DifferentiateAssistant::DifferentiateAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

DifferentiateAssistant::~DifferentiateAssistant()
{

}

void DifferentiateAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_differentiate_assistant.rc"));
    QAction* differentiate=new QAction(i18n("Differentiate"), actionCollection());
    differentiate->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("differentiate_assistant"), differentiate);
    connect(differentiate, &QAction::triggered, this, &DifferentiateAssistant::requested);
}

QStringList DifferentiateAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget* widget=new QWidget(dlg);
    Ui::DifferentiateAssistantBase base;
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
        int times=base.times->value();

        Cantor::CalculusExtension* ext=
            dynamic_cast<Cantor::CalculusExtension*>(backend()->extension(QLatin1String("CalculusExtension")));

        result<<ext->differentiate(expression, variable, times);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(differentiateassistant, "differentiateassistant.json", registerPlugin<DifferentiateAssistant>();)
#include "differentiateassistant.moc"
