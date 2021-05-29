/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "invertmatrixassistant.h"

#include <QAction>
#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
#include <KConfigGroup>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"
#include "ui_invertmatrixdlg.h"

InvertMatrixAssistant::InvertMatrixAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void InvertMatrixAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_invert_matrix_assistant.rc"));
    QAction* invertmatrix=new QAction(i18n("Invert Matrix"), actionCollection());
    actionCollection()->addAction(QLatin1String("invertmatrix_assistant"), invertmatrix);
    connect(invertmatrix, &QAction::triggered, this, &InvertMatrixAssistant::requested);
}

QStringList InvertMatrixAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    QWidget* widget=new QWidget(dlg);
    Ui::InvertMatrixAssistantBase base;
    base.setupUi(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(base.buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

    Cantor::HistoryExtension* hist=
        dynamic_cast<Cantor::HistoryExtension*>(backend()->extension(QLatin1String("HistoryExtension")));
    base.matrix->setText(hist->lastResult());

    QStringList result;
    if( dlg->exec())
    {
        const QString& m=base.matrix->text();
        Cantor::LinearAlgebraExtension* ext=
            dynamic_cast<Cantor::LinearAlgebraExtension*>(backend()->extension(QLatin1String("LinearAlgebraExtension")));
        result<<ext->invertMatrix(m);
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(invertmatrixassistant, "invertmatrixassistant.json", registerPlugin<InvertMatrixAssistant>();)
#include "invertmatrixassistant.moc"
