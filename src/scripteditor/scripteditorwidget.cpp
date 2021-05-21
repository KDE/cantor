/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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

ScriptEditorWidget::ScriptEditorWidget(const QString& filter, const QString& highlightingMode, QWidget* parent) : KXmlGuiWindow(parent),
m_filter(filter),
m_editor(nullptr),
m_script(nullptr),
m_tmpFile(nullptr)
{
    setObjectName(QStringLiteral("ScriptEditor"));

    KStandardAction::openNew(this, SLOT(newScript()), actionCollection());
    KStandardAction::open(this, SLOT(open()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());
    QAction * runAction = actionCollection()->addAction(QStringLiteral("file_execute"), this, SLOT(run()));
    runAction->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
    runAction->setText(i18n("Run Script"));

    KTextEditor::Editor* editor = KTextEditor::Editor::instance();
    if (!editor)
    {
        KMessageBox::error(this,  i18n("A KDE text-editor component could not be found;\n"
                                       "please check your KDE installation."));
    }
    else
    {
        m_script=editor->createDocument(nullptr);
        m_editor=qobject_cast<KTextEditor::View*>(m_script->createView(this));

        m_script->setHighlightingMode(highlightingMode);

        KConfigGroup cg(KSharedConfig::openConfig(), "ScriptEditor");
        setAutoSaveSettings(cg, true);

        setCentralWidget(m_editor);
        setupGUI(QSize(500,600), Default, QStringLiteral("cantor_scripteditor.rc"));
        guiFactory()->addClient(m_editor);

        KWindowConfig::restoreWindowSize(this->windowHandle(), cg);

        connect(m_script, &KTextEditor::Document::modifiedChanged, this, &ScriptEditorWidget::updateCaption);
        connect(m_script, &KTextEditor::Document::documentUrlChanged, this, &ScriptEditorWidget::updateCaption);
        updateCaption();
    }
}

ScriptEditorWidget::~ScriptEditorWidget()
{
    if (m_script)
        delete m_script;
    if (m_tmpFile)
        delete m_tmpFile;
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

void ScriptEditorWidget::open(const QUrl &url)
{
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


