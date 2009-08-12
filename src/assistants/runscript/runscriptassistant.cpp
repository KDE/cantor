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

#include "runscriptassistant.h"

#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kfiledialog.h>
#include "mathematik_macros.h"
#include "backend.h"
#include "extension.h"

RunScriptAssistant::RunScriptAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

RunScriptAssistant::~RunScriptAssistant()
{

}

void RunScriptAssistant::initActions()
{
    setXMLFile("mathematik_runscript_assistant.rc");
    KAction* runscript=new KAction(i18n("Run Script"), actionCollection());
    runscript->setIcon(KIcon(icon()));
    actionCollection()->addAction("runscript_assistant", runscript);
    connect(runscript, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList RunScriptAssistant::run(QWidget* parent)
{
    MathematiK::ScriptExtension* ext= dynamic_cast<MathematiK::ScriptExtension*>(backend()->extension("ScriptExtension"));
    QString file=KFileDialog::getOpenFileName(KUrl("kfiledialog://mathematik_script"), ext->scriptFileFilter(), parent);

    if(file.isNull())
    {
        return QStringList();
    }else
    {
        return QStringList()<<ext->runExternalScript(file);
    }
}

K_EXPORT_MATHEMATIK_PLUGIN(runscriptassistant, RunScriptAssistant)
