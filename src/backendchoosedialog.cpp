/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
*/

#include "backendchoosedialog.h"
#include "lib/backend.h"
#include "settings.h"

#include <KIconLoader>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QIcon>
#include <QPushButton>
#include <QWindow>

BackendChooseDialog::BackendChooseDialog(QWidget* parent) : QDialog(parent)
{
    QWidget* w = new QWidget(this);
    m_ui.setupUi(w);

    QGridLayout* layout = new QGridLayout;
    setLayout(layout);
    layout->addWidget(w);

    m_ui.backendList->setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    m_ui.backendList->setSortingEnabled(true);
    connect(m_ui.backendList, &QListWidget::currentItemChanged, this, &BackendChooseDialog::updateContent);
    connect(m_ui.backendList, &QListWidget::itemDoubleClicked, this, [=]() {
        if (m_ui.buttonBox->button(QDialogButtonBox::Ok)->isEnabled())
            accept();
    });

    m_ui.buttonBox->button(QDialogButtonBox::Ok);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    m_ui.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    for (auto* backend : Cantor::Backend::availableBackends())
    {
        qDebug() << backend->name() << backend->isEnabled() << backend->requirementsFullfilled();
        if(!backend->isEnabled())
            if (backend->requirementsFullfilled())
                continue;

        QListWidgetItem* item = new QListWidgetItem(m_ui.backendList);
        item->setText(backend->name());
        item->setIcon(QIcon::fromTheme(backend->icon()));
        m_ui.backendList->addItem(item);
        if(m_ui.backendList->currentItem() == nullptr)
            m_ui.backendList->setCurrentItem(item);

        if(backend->name()==Settings::self()->defaultBackend())
            m_ui.backendList->setCurrentItem(item);
    }

    int height = m_ui.backendList->iconSize().height() * m_ui.backendList->count();
    m_ui.backendList->setMinimumSize(0, height);

    setWindowTitle(i18n("Select the Backend"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("run-build")));

    connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, &BackendChooseDialog::accept);
    connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, &BackendChooseDialog::close);
    connect(this, &BackendChooseDialog::accepted, this, &BackendChooseDialog::onAccept);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "BackendChooseDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(500, 200).expandedTo(minimumSize()));
}

BackendChooseDialog::~BackendChooseDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "BackendChooseDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void BackendChooseDialog::onAccept()
{
    m_backend = m_ui.backendList->currentItem()->text();
    if(m_ui.makeDefault->isChecked())
    {
        Settings::self()->setDefaultBackend(m_backend);
        Settings::self()->save();
    }
}

void BackendChooseDialog::updateContent()
{
    auto* current = Cantor::Backend::getBackend( m_ui.backendList->currentItem()->text() );
    if (current)
    {
        QString desc;
        QString reason;
        QString header = i18n("<h1>%1</h1>"
                              "<div><b>Recommended version:</b> %2</div>",
                              current->name(), current->version());
        QString info = i18n("<hr><div>%1</div><br>"
                            "<div>See <a href=\"%2\">%2</a> for more information.</div>",
                            current->description(), current->url());

        if (current->requirementsFullfilled(&reason))
        {
            desc = header + info;
            m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            m_ui.makeDefault->setEnabled(true);
        }
        else
        {
            QString reasonMsg = i18n("<hr><div><b><font color='#e50000'>Some requirements are not fulfilled: </font></b>%1</div>", reason);
            desc = header + reasonMsg + info;
            m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            m_ui.makeDefault->setEnabled(false);
        }
        m_ui.descriptionView->setHtml(desc);
    }
}

QString BackendChooseDialog::backendName()
{
    return m_backend;
}
