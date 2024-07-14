/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "invertmatrixassistant.h"

#include <QDialog>
#include <QPushButton>
#include <QStyle>

#include <KActionCollection>
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
    auto* invertmatrix = new QAction(i18n("Invert Matrix"), actionCollection());
    actionCollection()->addAction(QLatin1String("invertmatrix_assistant"), invertmatrix);
    connect(invertmatrix, &QAction::triggered, this, &InvertMatrixAssistant::requested);
}

QStringList InvertMatrixAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg=new QDialog(parent);
    auto* widget = new QWidget(dlg);
    Ui::InvertMatrixAssistantBase base;
    base.setupUi(widget);
    auto* mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(base.buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    auto* hist = dynamic_cast<Cantor::HistoryExtension*>(backend()->extension(QLatin1String("HistoryExtension")));
    if (hist)
        base.matrix->setText(hist->lastResult());

    QStringList result;
    if( dlg->exec())
    {
        auto* ext = dynamic_cast<Cantor::LinearAlgebraExtension*>(backend()->extension(QLatin1String("LinearAlgebraExtension")));
        if (ext)
            result << ext->invertMatrix(base.matrix->text());
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(invertmatrixassistant, "invertmatrixassistant.json", registerPlugin<InvertMatrixAssistant>();)
#include "invertmatrixassistant.moc"
