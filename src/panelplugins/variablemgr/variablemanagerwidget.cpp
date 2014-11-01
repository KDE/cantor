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
    Copyright (C) 2010 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "variablemanagerwidget.h"

#include <QLayout>
#include <QTreeView>
#include <QToolButton>
#include <QAbstractItemModel>

#include <KDialog>
#include <KIcon>
#include <KLocale>
#include <QDebug>
#include <KIconLoader>
#include <KFileDialog>
#include <KMessageBox>

#include "session.h"
#include "extension.h"
#include "backend.h"

#include "ui_newvardlg.h"

VariableManagerWidget::VariableManagerWidget(Cantor::Session* session, QWidget* parent) : QWidget(parent)
{
    m_session=0;

    QVBoxLayout* layout=new QVBoxLayout(this);

    m_table=new QTreeView(this);
    m_table->setRootIsDecorated(false);

    layout->addWidget(m_table, 1);

    QHBoxLayout* btnLayout=new QHBoxLayout(this);
    int size=KIconLoader::global()->currentSize(KIconLoader::MainToolbar);

    m_newBtn=new QToolButton(this);
    m_newBtn->setIcon(KIcon(QLatin1String("document-new")));
    m_newBtn->setToolTip(i18n("Add new variable"));
    m_newBtn->setIconSize(QSize(size, size));
    connect(m_newBtn, SIGNAL(clicked()), this, SLOT(newVariable()));
    btnLayout->addWidget(m_newBtn);


    m_loadBtn=new QToolButton(this);
    m_loadBtn->setIcon(KIcon(QLatin1String("document-open")));
    m_loadBtn->setToolTip(i18n("Load Variables"));
    m_loadBtn->setIconSize(QSize(size, size));
    connect(m_loadBtn, SIGNAL(clicked()), this, SLOT(load()));
    btnLayout->addWidget(m_loadBtn);

    m_saveBtn=new QToolButton(this);
    m_saveBtn->setIcon(KIcon(QLatin1String("document-save")));
    m_saveBtn->setToolTip(i18n("Store Variables"));
    m_saveBtn->setIconSize(QSize(size, size));
    connect(m_saveBtn, SIGNAL(clicked()), this, SLOT(save()));
    btnLayout->addWidget(m_saveBtn);

    m_clearBtn=new QToolButton(this);
    m_clearBtn->setIcon(KIcon(QLatin1String("edit-clear")));
    m_clearBtn->setToolTip(i18n("Clear Variables"));
    m_clearBtn->setIconSize(QSize(size, size));
    connect(m_clearBtn, SIGNAL(clicked()), this, SLOT(clearVariables()));
    btnLayout->addWidget(m_clearBtn);

    layout->addLayout(btnLayout);

    setLayout(layout);

    setSession(session);

    //check for the methods the backend actually supports, and disable the buttons accordingly
    Cantor::VariableManagementExtension* ext=
        dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));
    if(ext->loadVariables(QString()).isNull())
        m_loadBtn->setDisabled(true);
    if(ext->saveVariables(QString()).isNull())
        m_saveBtn->setDisabled(true);
    if(ext->addVariable(QString(), QString()).isNull())
        m_newBtn->setDisabled(true);
    if(ext->clearVariables().isNull())
        m_clearBtn->setDisabled(true);
}

VariableManagerWidget::~VariableManagerWidget()
{

}

void VariableManagerWidget::setSession(Cantor::Session* session)
{
    qDebug()<<"setting session to "<<session;
    m_session=session;
    if(session)
    {
        m_model=session->variableModel();
        qDebug()<<"model: "<<m_model;
        if(m_table)
            m_table->setModel(m_model);
    }
}

void VariableManagerWidget::clearVariables()
{
    int btn=KMessageBox::questionYesNo(this,  i18n("Are you sure you want to remove all variables?"), i18n("Confirmation - Cantor"));
    if(btn==KMessageBox::Yes)
    {
        qDebug()<<"removing it all";
        m_model->removeRows(0, m_model->rowCount());

        //evaluate the "clear" command
        Cantor::VariableManagementExtension* ext=
            dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));
        const QString& cmd=ext->clearVariables();
        emit runCommand(cmd);

        //HACK? should the model detect that this happened on its own?
        //inform the model that all variables have been removed.
        //Do so by trying to evaluate the clearVariables slot of
        //DefaultVariableModel. If our model isn't one of those,
        //this call will just do nothing.
        QMetaObject::invokeMethod(m_model,  "clearVariables", Qt::QueuedConnection);
    }
}

void VariableManagerWidget::save()
{
    const QString file=KFileDialog::getSaveFileName(KUrl(),  QString(),  this);

    Cantor::VariableManagementExtension* ext=
        dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));

    const QString& cmd=ext->saveVariables(file);
    emit runCommand(cmd);
}

void VariableManagerWidget::load()
{
    const QString file=KFileDialog::getOpenFileName(KUrl(),  QString(),  this);

    Cantor::VariableManagementExtension* ext=
        dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));

    const QString& cmd=ext->loadVariables(file);
    emit runCommand(cmd);
}

void VariableManagerWidget::newVariable()
{
    QPointer<KDialog> dlg=new KDialog(this);
    QWidget *widget=new QWidget(dlg);
    Ui::NewVariableDialogBase base;
    base.setupUi(widget);
    dlg->setMainWidget(widget);

    if( dlg->exec())
    {
        const QString& name=base.name->text();
        const QString& val=base.value->text();

        Cantor::VariableManagementExtension* ext=
            dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));

        const QString& cmd=ext->addVariable(name, val);

        emit runCommand(cmd);
    }

    delete dlg;

}
