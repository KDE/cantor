/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "creatematrixassistant.h"

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
    auto* creatematrix = new QAction(i18n("Create Matrix"), actionCollection());
    actionCollection()->addAction(QLatin1String("creatematrix_assistant"), creatematrix);
    connect(creatematrix, &QAction::triggered, this, &CreateMatrixAssistant::requested);
}

QStringList CreateMatrixAssistant::run(QWidget* parent)
{
    QPointer<CreateMatrixDlg> dlg = new CreateMatrixDlg(parent);

    QStringList result;
    if (dlg->exec())
    {
        Cantor::LinearAlgebraExtension::Matrix m;
        for (int i = 0; i < dlg->numRows(); i++)
        {
            QStringList row;
            for(int j=0; j<dlg->numCols(); j++)
                row << dlg->value(i, j);
             m << row;
        }

        auto* ext = dynamic_cast<Cantor::LinearAlgebraExtension*>(backend()->extension(QLatin1String("LinearAlgebraExtension")));
        if (ext)
            result<<ext->createMatrix(m);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(creatematrixassistant, "creatematrixassistant.json", registerPlugin<CreateMatrixAssistant>();)
#include "creatematrixassistant.moc"
