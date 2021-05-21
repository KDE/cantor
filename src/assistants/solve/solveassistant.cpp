/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
