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

#include <QTemporaryFile>

#include <QUrl>

#include <KLocalizedString>
#include <QDebug>
#include <KMessageBox>
#include <QFileDialog>
#include <KWindowConfig>
#include <QAction>
#include <KStandardAction>
#include <KActionCollection>
#include <KXMLGUIFactory>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
// #include <KTextEditor/EditorChooser>
#include <QFileDialog>

ScriptEditorWidget::ScriptEditorWidget(const QString& filter, const QString& highlightingMode, QWidget* parent) : KXmlGuiWindow(parent)
{
    setObjectName(QLatin1String("ScriptEditor"));

    m_filter=filter;
    m_tmpFile=nullptr;

    KStandardAction::openNew(this, SLOT(newScript()), actionCollection());
    KStandardAction::open(this, SLOT(open()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());
    QAction * runAction = actionCollection()->addAction(QLatin1String("file_execute"), this, SLOT(run()));
    runAction->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    runAction->setText(i18n("Run Script"));

    KTextEditor::Editor* editor = KTextEditor::Editor::instance();
    if (!editor)
    {
        KMessageBox::error(this,  i18n("A KDE text-editor component could not be found;\n"
                                       "please check your KDE installation."));
        m_script=nullptr;
    }
    else
    {
        m_script=editor->createDocument(nullptr);
        m_editor=qobject_cast<KTextEditor::View*>(m_script->createView(this));

        m_script->setHighlightingMode(highlightingMode);

        KConfigGroup cg(KSharedConfig::openConfig(), "ScriptEditor");
        setAutoSaveSettings(cg, true);

        setCentralWidget(m_editor);
        setupGUI(QSize(500,600), Default, QLatin1String("cantor_scripteditor.rc"));
        guiFactory()->addClient(m_editor);

        KWindowConfig::restoreWindowSize(this->windowHandle(), cg);

        connect(m_script, &KTextEditor::Document::modifiedChanged, this, &ScriptEditorWidget::updateCaption);
        connect(m_script, &KTextEditor::Document::documentUrlChanged, this, &ScriptEditorWidget::updateCaption);
        updateCaption();
    }
}

ScriptEditorWidget::~ScriptEditorWidget()
{
}

void ScriptEditorWidget::newScript()
{
    QString highlightingMode = m_script->highlightingMode();
    m_script->closeUrl();
    m_script->setHighlightingMode(highlightingMode);
}

void ScriptEditorWidget::open()
{
    QUrl url = QFileDialog::getOpenFileUrl(this, QString(), QUrl(), m_filter);

    m_script->openUrl(url);
}

void ScriptEditorWidget::run()
{
    QString filename;
    if(!m_script->url().isLocalFile())
    {
        // If the script is not in a local file, write it to a temporary file
        if(m_tmpFile==nullptr)
        {
            m_tmpFile=new QTemporaryFile();
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

    qDebug()<<"running "<<filename;
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


