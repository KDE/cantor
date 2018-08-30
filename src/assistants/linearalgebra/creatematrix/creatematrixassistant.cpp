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

#include "creatematrixassistant.h"

#include <QAction>

#include <QDialog>
#include <QPushButton>
#include <KActionCollection>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"
#include "creatematrixdlg.h"

CreateMatrixAssistant::CreateMatrixAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void CreateMatrixAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_create_matrix_assistant.rc"));
    QAction* creatematrix=new QAction(i18n("Create Matrix"), actionCollection());
    actionCollection()->addAction(QLatin1String("creatematrix_assistant"), creatematrix);
    connect(creatematrix, &QAction::triggered, this, &CreateMatrixAssistant::requested);
}

QStringList CreateMatrixAssistant::run(QWidget* parent)
{
    QPointer<CreateMatrixDlg> dlg=new CreateMatrixDlg(parent);

    QStringList result;
    if( dlg->exec())
    {
        Cantor::LinearAlgebraExtension::Matrix m;
        for (int i=0;i<dlg->numRows();i++)
        {
            QStringList row;
            for(int j=0;j<dlg->numCols();j++)
                row<<dlg->value(i, j);
             m<<row;
        }

        Cantor::LinearAlgebraExtension* ext=
            dynamic_cast<Cantor::LinearAlgebraExtension*>(backend()->extension(QLatin1String("LinearAlgebraExtension")));
        result<<ext->createMatrix(m);
    }


    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(creatematrixassistant, "creatematrixassistant.json", registerPlugin<CreateMatrixAssistant>();)
#include "creatematrixassistant.moc"
