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

#include "solveassistant.h"

#include <QAction>
#include <QIcon>
#include <QStyle>
#include <QDialog>
#include <KActionCollection>
#include <KConfigGroup>
#include <QPushButton>
#include "ui_solvedlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

SolveAssistant::SolveAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void SolveAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_solve_assistant.rc"));
    QAction* solve=new QAction(i18n("Solve equations"), actionCollection());
    solve->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("solve_assistant"), solve);
    connect(solve, &QAction::triggered, this, &SolveAssistant::requested);
}

QStringList SolveAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::SolveAssistantBase base;
    base.setupUi(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    connect(base.buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()) );
    connect(base.buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()) );

    QStringList result;
    if( dlg->exec())
    {
        QStringList equations=base.equations->toPlainText().split(QLatin1Char('\n'));
        QStringList variables=base.variables->text().split(QLatin1String(", "));

        Cantor::CASExtension* ext=
            dynamic_cast<Cantor::CASExtension*>(backend()->extension(QLatin1String("CASExtension")));

        result<<ext->solve(equations, variables);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(solveassistant, "solveassistant.json", registerPlugin<SolveAssistant>();)
#include "solveassistant.moc"
