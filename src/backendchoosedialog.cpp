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

#include <KLocalizedString>
#include <QIcon>
#include <QPushButton>
#include <KIconLoader>

#include "lib/backend.h"
#include "settings.h"

const char* BackendChooseDialog::descriptionTemplate = I18N_NOOP("<h1>%1</h1>" \
                                                                 "<div><b>Recommended version:</b> %4</div><br/>" \
                                                                 "<div>%2</div><br/>" \
                                                                 "<div>See <a href=\"%3\">%3</a> for more information</div>");
const char* BackendChooseDialog::requirementsTemplate= I18N_NOOP("<h1>%1</h1>"
                                                                 "<div><b>Recommended version:</b> %3</div><br/>"
                                                                 "<div>Some requirements for the backend don't fulfilled:<br/>%2</div><br/>");

BackendChooseDialog::BackendChooseDialog(QWidget* parent) : QDialog(parent)
{
    QWidget* w=new QWidget(this);
    m_ui.setupUi(w);

    QGridLayout *layout = new QGridLayout;
    setLayout(layout);
    layout->addWidget(w);

    m_ui.backendList->setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    m_ui.backendList->setSortingEnabled(true);
    connect(m_ui.backendList, &QListWidget::currentItemChanged, this, &BackendChooseDialog::updateContent);
    connect(m_ui.backendList, &QListWidget::itemDoubleClicked, this, &BackendChooseDialog::accept);

    m_ui.buttonBox->button(QDialogButtonBox::Ok);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    m_ui.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    foreach(Cantor::Backend* backend,  Cantor::Backend::availableBackends())
    {
        qDebug() << backend->name() << backend->isEnabled() << backend->requirementsFullfilled();
        if(!backend->isEnabled())
            if (backend->requirementsFullfilled())
                continue;

        QListWidgetItem* item=new QListWidgetItem(m_ui.backendList);
        item->setText(backend->name());
        item->setIcon(QIcon::fromTheme(backend->icon()));
        m_ui.backendList->addItem(item);
        if(m_ui.backendList->currentItem() == nullptr)
            m_ui.backendList->setCurrentItem(item);

        if(backend->name()==Settings::self()->defaultBackend())
            m_ui.backendList->setCurrentItem(item);
    }

    connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, &BackendChooseDialog::accept);
    connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, &BackendChooseDialog::close);

    connect(this, &BackendChooseDialog::accepted, this, &BackendChooseDialog::onAccept);
}

void BackendChooseDialog::onAccept()
{
    m_backend=m_ui.backendList->currentItem()->text();
    if(m_ui.makeDefault->isChecked())
    {
        Settings::self()->setDefaultBackend(m_backend);
        Settings::self()->save();
    }
}

void BackendChooseDialog::updateContent()
{
    Cantor::Backend* current=Cantor::Backend::getBackend( m_ui.backendList->currentItem()->text() );
    if (current)
    {
        QString desc;
        QString reason;
        if (current->requirementsFullfilled(&reason))
        {
            desc = i18n(BackendChooseDialog::descriptionTemplate,
                                    current->name(), current->description(), current->url(), current->version());
            m_ui.buttonBox->setEnabled(true);
            m_ui.makeDefault->setEnabled(true);
        }
        else
        {
            desc = i18n(BackendChooseDialog::requirementsTemplate,
                                    current->name(), reason, current->version());
            m_ui.buttonBox->setEnabled(false);
            m_ui.makeDefault->setEnabled(false);
        }
        m_ui.descriptionView->setHtml(desc);
    }
}

QString BackendChooseDialog::backendName()
{
    return m_backend;
}
