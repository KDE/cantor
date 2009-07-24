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

#include <kdialog.h>
#include <kaction.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include "mathematik_macros.h"
#include "backend.h"
#include "extension.h"
#include "creatematrixdlg.h"

CreateMatrixAssistant::CreateMatrixAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

CreateMatrixAssistant::~CreateMatrixAssistant()
{

}

void CreateMatrixAssistant::initActions()
{
    setXMLFile("mathematik_create_matrix_assistant.rc");
    KAction* creatematrix=new KAction(i18n("Create Matrix"), actionCollection());
    creatematrix->setIcon(KIcon(icon()));
    actionCollection()->addAction("creatematrix_assistant", creatematrix);
    connect(creatematrix, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList CreateMatrixAssistant::run(QWidget* parent)
{
    CreateMatrixDlg dlg(parent);

    if( dlg.exec())
    {
        MathematiK::LinearAlgebraExtension::Matrix m;
        for (int i=0;i<dlg.numRows();i++)
        {
            QStringList row;
            for(int j=0;j<dlg.numCols();j++)
                row<<dlg.value(i, j);
             m<<row;
        }

        MathematiK::LinearAlgebraExtension* ext= dynamic_cast<MathematiK::LinearAlgebraExtension*>(backend()->extension("LinearAlgebraExtension"));
        return QStringList()<<ext->createMatrix(m);
    }
    return QStringList();
}

K_EXPORT_MATHEMATIK_PLUGIN(creatematrixassistant, CreateMatrixAssistant)
