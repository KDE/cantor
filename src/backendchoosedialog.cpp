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

#include "backendchoosedialog.h"

#include "lib/backend.h"

BackendChooseDialog::BackendChooseDialog(QWidget* parent) : KDialog(parent)
{
    QWidget* w=new QWidget(this);
    m_ui.setupUi(w);
    connect(m_ui.backendList, SIGNAL(currentItemChanged ( QListWidgetItem *,  QListWidgetItem *)), this, SLOT(accept()));
    foreach(MathematiK::Backend* backend,  MathematiK::Backend::availableBackends())
    {
        if(!backend->isEnabled()) //don't show disabled backends
            continue;

        QListWidgetItem* item=new QListWidgetItem(m_ui.backendList);
        item->setText(backend->name());
        item->setIcon(KIcon(backend->icon()));
        item->setToolTip(backend->description());
        m_ui.backendList->addItem(item);
    }

    setMainWidget(w);
    connect(this, SIGNAL(accepted()), this, SLOT(onAccept()));
}

BackendChooseDialog::~BackendChooseDialog()
{
}

void BackendChooseDialog::onAccept()
{
    m_backend=m_ui.backendList->currentItem()->text();
}

QString BackendChooseDialog::backendName()
{
    return m_backend;
}

#include "backendchoosedialog.moc"
