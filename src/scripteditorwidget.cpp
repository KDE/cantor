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

#include <QGridLayout>
#include <QPushButton>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>


ScriptEditorWidget::ScriptEditorWidget(const QString& filter, QWidget* parent) : QWidget(parent)
{
    m_filter=filter;
    m_tmpFile=0;
    m_layout=new QGridLayout();

    //create the buttons
    QPushButton* newBtn=new QPushButton(KIcon("document-new"), i18n("New"), this);
    QPushButton* openBtn=new QPushButton(KIcon("document-open"), i18n("Open"), this);
    QPushButton* saveBtn=new QPushButton(KIcon("document-save"), i18n("Save"), this);
    QPushButton* runBtn=new QPushButton(KIcon("system-run"), i18n("Run Script"), this);

    connect(newBtn, SIGNAL(clicked()), this, SLOT(newScript()));
    connect(openBtn, SIGNAL(clicked()), this, SLOT(open()));
    connect(saveBtn, SIGNAL(clicked()), this, SLOT(save()));
    connect(runBtn, SIGNAL(clicked()), this, SLOT(run()));

    m_layout->addWidget(newBtn,  0, 0);
    m_layout->addWidget(openBtn, 0, 1);
    m_layout->addWidget(saveBtn, 0, 2);
    m_layout->addWidget(runBtn,  0, 3);

    m_script=0;
    m_editor=0;
    newScript();

    setLayout(m_layout);

}

ScriptEditorWidget::~ScriptEditorWidget()
{

}

void ScriptEditorWidget::newScript()
{
    if(m_script)
    {
        m_script->deleteLater();
        m_editor->deleteLater();
    }

    //Get a text editor
    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    if (!editor)
    {
        KMessageBox::error(this,  i18n("A KDE text-editor component could not be found;\n"
                                           "please check your KDE installation."));
    }

    m_script = editor->createDocument(0);
    m_editor=qobject_cast<KTextEditor::View*>(m_script->createView(this));
    m_layout->addWidget(m_editor, 1, 0, 1, 4);

    m_editor->setMinimumHeight(500);
    setTabOrder(0, m_editor);
}

void ScriptEditorWidget::open()
{
    KUrl url=KFileDialog::getOpenFileName(KUrl("kfiledialog://cantor_script"), m_filter, this);
    m_script->openUrl(url);
}

void ScriptEditorWidget::save()
{
    m_script->documentSave();
}

void ScriptEditorWidget::run()
{
    QString filename;
    if(m_script->url().isEmpty())
    {
        //Write the stuff to a temporary file
        if(m_tmpFile==0)
        {
            m_tmpFile=new KTemporaryFile();
            m_tmpFile->setPrefix( "cantor/" );
        }

        m_tmpFile->open();
        QString text=m_script->text();
        m_tmpFile->write(text.toUtf8());
        m_tmpFile->close();

        filename=m_tmpFile->fileName();
    }else
    {
        filename=m_script->url().toLocalFile();
    }

    kDebug()<<"running "<<filename;
    emit runScript(filename);
}

#include "scripteditorwidget.moc"
