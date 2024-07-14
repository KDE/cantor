/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "solveassistant.h"

#include <QIcon>
#include <QStyle>
#include <QDialog>
#include <QPushButton>

#include <KActionCollection>
#include <KConfigGroup>
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
    auto* solve = new QAction(i18n("Solve equations"), actionCollection());
    solve->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("solve_assistant"), solve);
    connect(solve, &QAction::triggered, this, &SolveAssistant::requested);
}

QStringList SolveAssistant::run(QWidget* parent)
{
    QPointer<QDialog> dlg = new QDialog(parent);
    auto* widget = new QWidget(dlg);
    Ui::SolveAssistantBase base;
    base.setupUi(widget);
    auto* mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(widget);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(base.buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(base.buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    QStringList result;
    if( dlg->exec())
    {
        auto* ext = dynamic_cast<Cantor::CASExtension*>(backend()->extension(QLatin1String("CASExtension")));
        if (ext)
        {
            const auto& equations = base.equations->toPlainText().split(QLatin1Char('\n'));
            const auto& variables = base.variables->text().split(QLatin1String(", "));
            result<<ext->solve(equations, variables);
        }
    }

    delete dlg;
    return result;
}

K_PLUGIN_FACTORY_WITH_JSON(solveassistant, "solveassistant.json", registerPlugin<SolveAssistant>();)
#include "solveassistant.moc"
