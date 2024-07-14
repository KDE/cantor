/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "runscriptassistant.h"

#include <QIcon>
#include <QDialog>

#include <KActionCollection>
#include <QFileDialog>
#include <KLocalizedString>
#include "cantor_macros.h"
#include "backend.h"
#include "extension.h"

RunScriptAssistant::RunScriptAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

void RunScriptAssistant::initActions()
{
    setXMLFile(QLatin1String("cantor_runscript_assistant.rc"));
    auto* runscript = new QAction(i18n("Run Script"), actionCollection());
    runscript->setIcon(QIcon::fromTheme(icon()));
    actionCollection()->addAction(QLatin1String("runscript_assistant"), runscript);
    connect(runscript, &QAction::triggered, this, &RunScriptAssistant::requested);
}

QStringList RunScriptAssistant::run(QWidget* parent)
{
    auto* ext = dynamic_cast<Cantor::ScriptExtension*>(backend()->extension(QLatin1String("ScriptExtension")));
    if (!ext)
        return QStringList();

    const auto& file = QFileDialog::getOpenFileName(parent, QString(), QLatin1String("qfiledialog://cantor_script"), ext->scriptFileFilter());
    if(!file.isNull())
        return {ext->runExternalScript(file)};
    else
        return QStringList();
}

K_PLUGIN_FACTORY_WITH_JSON(runscriptassistant, "runscriptassistant.json", registerPlugin<RunScriptAssistant>();)
#include "runscriptassistant.moc"
