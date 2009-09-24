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

#include "eigenvaluesassistant.h"

#include <kdialog.h>
#include <kaction.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"
#include "ui_eigenvaluesdlg.h"

EigenValuesAssistant::EigenValuesAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

EigenValuesAssistant::~EigenValuesAssistant()
{

}

void EigenValuesAssistant::initActions()
{
    setXMLFile("cantor_eigenvalues_assistant.rc");
    KAction* eigenvalues=new KAction(i18n("Compute Eigenvalues"), actionCollection());
    //eigenvalues->setIcon(KIcon(icon()));
    actionCollection()->addAction("eigenvalues_assistant", eigenvalues);
    connect(eigenvalues, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList EigenValuesAssistant::run(QWidget* parent)
{
    QPointer<KDialog> dlg=new KDialog(parent);
    QWidget* widget=new QWidget(dlg);
    Ui::EigenValuesAssistantBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

    Cantor::HistoryExtension* hist= dynamic_cast<Cantor::HistoryExtension*>(backend()->extension("HistoryExtension"));
    base.matrix->setText(hist->lastResult());

    QStringList result;
    if( dlg->exec())
    {
        const QString& m=base.matrix->text();
        Cantor::LinearAlgebraExtension* ext= dynamic_cast<Cantor::LinearAlgebraExtension*>(backend()->extension("LinearAlgebraExtension"));
        result<<ext->eigenValues(m);
    }

    delete dlg;
    return result;
}

K_EXPORT_CANTOR_PLUGIN(eigenvaluesassistant, EigenValuesAssistant)
