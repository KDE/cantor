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

#include "scripteditorwidget.h"

#include <ktemporaryfile.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>

ScriptEditorWidget::ScriptEditorWidget(const QString& filter, const QString& highlightingMode, QWidget* parent) : KXmlGuiWindow(parent)
{
    setObjectName("ScriptEditor");

    m_filter=filter;
    m_tmpFile=0;

    KStandardAction::openNew(this, SLOT(newScript()), actionCollection());
    KStandardAction::open(this, SLOT(open()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());
    KAction* runAction = actionCollection()->addAction("file_execute", this, SLOT(run()));
    runAction->setIcon(KIcon("system-run"));
    runAction->setText(i18n("Run Script"));

    KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
    if (!editor)
    {
        KMessageBox::error(this,  i18n("A KDE text-editor component could not be found;\n"
                                       "please check your KDE installation."));
        m_script=0;
    }
    else
    {
        m_script=editor->createDocument(0);
        m_editor=qobject_cast<KTextEditor::View*>(m_script->createView(this));

        m_script->setHighlightingMode(highlightingMode);

        KConfigGroup cg(KGlobal::config(), "ScriptEditor");
        setAutoSaveSettings(cg, true);

        setCentralWidget(m_editor);
        setupGUI(QSize(500,600), Default, "cantor_scripteditor.rc");
        guiFactory()->addClient(m_editor);
        restoreWindowSize(cg);

        connect(m_script, SIGNAL(modifiedChanged(KTextEditor::Document*)), this, SLOT(updateCaption()));
        connect(m_script, SIGNAL(documentUrlChanged(KTextEditor::Document*)), this, SLOT(updateCaption()));
        updateCaption();
    }
}

ScriptEditorWidget::~ScriptEditorWidget()
{
}

void ScriptEditorWidget::newScript()
{
    m_script->closeUrl();
}

void ScriptEditorWidget::open()
{
    KUrl url=KFileDialog::getOpenFileName(KUrl("kfiledialog://cantor_script"), m_filter, this);
    m_script->openUrl(url);
}

void ScriptEditorWidget::run()
{
    QString filename;
    if(!m_script->url().isLocalFile())
    {
        // If the script is not in a local file, write it to a temporary file
        if(m_tmpFile==0)
        {
            m_tmpFile=new KTemporaryFile();
        }
        else
        {
            m_tmpFile->resize(0);
        }
        m_tmpFile->open();
        QString text=m_script->text();
        m_tmpFile->write(text.toUtf8());
        m_tmpFile->close();

        filename=m_tmpFile->fileName();
    }else
    {
        m_script->save();
        filename=m_script->url().toLocalFile();
    }

    kDebug()<<"running "<<filename;
    emit runScript(filename);
}

bool ScriptEditorWidget::queryClose()
{
    if(m_script)
        return m_script->queryClose();
    else
        return true;
}

void ScriptEditorWidget::updateCaption()
{
    QString fileName = m_script->url().toLocalFile();
    bool modified = m_script->isModified();
    if (fileName.isEmpty())
    {
        setCaption(i18n("Script Editor"), modified);
    }else
    {
         setCaption(i18n("Script Editor - %1", fileName), modified);
    }
}

#include "scripteditorwidget.moc"
