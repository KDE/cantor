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

#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include "ui_solvedlg.h"
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

SolveAssistant::SolveAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

SolveAssistant::~SolveAssistant()
{

}

void SolveAssistant::initActions()
{
    setXMLFile("cantor_solve_assistant.rc");
    KAction* solve=new KAction(i18n("Solve equations"), actionCollection());
    solve->setIcon(KIcon(icon()));
    actionCollection()->addAction("solve_assistant", solve);
    connect(solve, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList SolveAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);
    QWidget *widget=new QWidget(dlg);
    Ui::SolveAssistantBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

    QStringList result;
    if( dlg->exec())
    {
        QStringList equations=base.equations->toPlainText().split('\n');
        QStringList variables=base.variables->text().split(", ");

        Cantor::CASExtension* ext= dynamic_cast<Cantor::CASExtension*>(backend()->extension("CASExtension"));

        result<<ext->solve(equations, variables);
    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(solveassistant, SolveAssistant)
