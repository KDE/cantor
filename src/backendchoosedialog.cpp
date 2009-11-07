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
#include "settings.h"

const char* BackendChooseDialog::descriptionTemplate = I18N_NOOP("<h1>%1</h1>" \
                                                                 "<div>%2</div><br/>" \
                                                                 "<div>See <a href=\"%3\">%3</a> for more information</div>");

BackendChooseDialog::BackendChooseDialog(QWidget* parent) : KDialog(parent)
{
    QWidget* w=new QWidget(this);
    m_ui.setupUi(w);
    m_ui.backendList->setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    connect(m_ui.backendList, SIGNAL(currentItemChanged ( QListWidgetItem *,  QListWidgetItem *)), this, SLOT(updateDescription()));
    connect(m_ui.backendList, SIGNAL(itemDoubleClicked( QListWidgetItem *)), this, SLOT(accept()));

    foreach(Cantor::Backend* backend,  Cantor::Backend::availableBackends())
    {
        if(!backend->isEnabled()) //don't show disabled backends
            continue;

        QListWidgetItem* item=new QListWidgetItem(m_ui.backendList);
        item->setText(backend->name());
        item->setIcon(KIcon(backend->icon()));
        m_ui.backendList->addItem(item);
        if(m_ui.backendList->currentItem() == 0)
            m_ui.backendList->setCurrentItem(item);

        if(backend->name()==Settings::self()->defaultBackend())
            m_ui.backendList->setCurrentItem(item);
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
    if(m_ui.makeDefault->isChecked())
        Settings::self()->setDefaultBackend(m_backend);
}

void BackendChooseDialog::updateDescription()
{
    Cantor::Backend* current=Cantor::Backend::createBackend( m_ui.backendList->currentItem()->text() );
    m_ui.descriptionView->setHtml(i18n(BackendChooseDialog::descriptionTemplate, current->name(), current->description(), current->url()));
}

QString BackendChooseDialog::backendName()
{
    return m_backend;
}

#include "backendchoosedialog.moc"
