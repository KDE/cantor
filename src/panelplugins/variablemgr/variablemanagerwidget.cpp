/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
*/

#include "variablemanagerwidget.h"
#include "backend.h"
#include "extension.h"
#include "session.h"

#include "ui_newvardlg.h"

#include <QAction>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDialog>
#include <QFileDialog>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>

#include <KIconLoader>
#include <KMessageBox>

VariableManagerWidget::VariableManagerWidget(Cantor::Session* session, QWidget* parent) : QWidget(parent),
    m_treeView(new QTreeView(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_treeView, 1);

    m_treeView->setRootIsDecorated(false);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setAlternatingRowColors(true);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(0);
    btnLayout->setMargin(0);

    //Buttons to save/load the variables
    int size = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);

    m_newBtn = new QToolButton(this);
    m_newBtn->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
    m_newBtn->setToolTip(i18n("Add New Variable"));
    m_newBtn->setIconSize(QSize(size, size));
    btnLayout->addWidget(m_newBtn);

    m_loadBtn=new QToolButton(this);
    m_loadBtn->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    m_loadBtn->setToolTip(i18n("Load Variables"));
    m_loadBtn->setIconSize(QSize(size, size));
    btnLayout->addWidget(m_loadBtn);

    m_saveBtn = new QToolButton(this);
    m_saveBtn->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    m_saveBtn->setToolTip(i18n("Save Variables"));
    m_saveBtn->setIconSize(QSize(size, size));
    btnLayout->addWidget(m_saveBtn);

    m_clearBtn = new QToolButton(this);
    m_clearBtn->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    m_clearBtn->setToolTip(i18n("Remove Variables"));
    m_clearBtn->setIconSize(QSize(size, size));
    btnLayout->addWidget(m_clearBtn);

    auto* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    btnLayout->addItem(spacer);

    //Filter
    m_leFilter = new QLineEdit(this);
    m_leFilter->setClearButtonEnabled(true);
    m_leFilter->setPlaceholderText(i18n("Search/Filter"));
    btnLayout->addWidget(m_leFilter);

    m_bFilterOptions = new QToolButton(this);
    m_bFilterOptions->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    m_bFilterOptions->setCheckable(true);
    btnLayout->addWidget(m_bFilterOptions);

    layout->addLayout(btnLayout);

    //actions
    m_caseSensitiveAction = new QAction(i18n("Case Sensitive"), this);
    m_caseSensitiveAction->setCheckable(true);
    m_caseSensitiveAction->setChecked(false);

    m_matchCompleteWordAction = new QAction(i18n("Match Complete Word"), this);
    m_matchCompleteWordAction->setCheckable(true);
    m_matchCompleteWordAction->setChecked(false);

    //signal-slot connections
    connect(m_leFilter, &QLineEdit::textChanged, this, &VariableManagerWidget::filterTextChanged);
    connect(m_bFilterOptions, &QPushButton::toggled, this, &VariableManagerWidget::toggleFilterOptionsMenu);
    connect(m_caseSensitiveAction, &QAction::triggered, this, [=]() {filterTextChanged(m_leFilter->text());} );
    connect(m_matchCompleteWordAction, &QAction::triggered, this, [=]() {filterTextChanged(m_leFilter->text());});
    connect(m_newBtn, &QToolButton::clicked, this, &VariableManagerWidget::newVariable);
    connect(m_loadBtn, &QToolButton::clicked, this, &VariableManagerWidget::load);
    connect(m_saveBtn, &QToolButton::clicked, this, &VariableManagerWidget::save);
    connect(m_clearBtn, &QToolButton::clicked, this, &VariableManagerWidget::clearVariables);

    setSession(session);
}

void VariableManagerWidget::setSession(Cantor::Session* session)
{
    m_session = session;
    if (session)
    {
        m_model = session->variableDataModel();
        if (m_treeView)
            m_treeView->setModel(m_model);

        connect(m_model, &QAbstractItemModel::rowsInserted, this, &VariableManagerWidget::updateButtons);
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, &VariableManagerWidget::updateButtons);
        updateButtons();

        //check for the methods the backend actually supports, and disable the buttons accordingly
        auto* ext = dynamic_cast<Cantor::VariableManagementExtension*>(
            m_session->backend()->extension(QLatin1String("VariableManagementExtension"))
        );

        if (ext)
        {
            if(ext->loadVariables(QString()).isNull())
                m_loadBtn->setDisabled(true);
            if(ext->saveVariables(QString()).isNull())
                m_saveBtn->setDisabled(true);
            if(ext->addVariable(QString(), QString()).isNull())
                m_newBtn->setDisabled(true);
            if(ext->clearVariables().isNull())
                m_clearBtn->setDisabled(true);
        }
    }
}

void VariableManagerWidget::clearVariables()
{
    int btn = KMessageBox::questionYesNo(this,
                                         i18n("Are you sure you want to remove all variables?"),
                                         i18n("Remove Variables"));
    if (btn == KMessageBox::Yes)
    {
        m_model->removeRows(0, m_model->rowCount());

        //evaluate the "clear" command
        auto* ext = dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));
        if (ext)
        {
            const QString& cmd = ext->clearVariables();
            emit runCommand(cmd);
        }

        //HACK? should the model detect that this happened on its own?
        //inform the model that all variables have been removed.
        //Do so by trying to evaluate the clearVariables slot of
        //DefaultVariableModel. If our model isn't one of those,
        //this call will just do nothing.
        QMetaObject::invokeMethod(m_model,  "clearVariables", Qt::QueuedConnection);

        //QAbstractItemModel::rowsRemoved() doesn't seem to be sent in this case,
        //call updateButtons explicitly
        QTimer::singleShot(0, this, [=] () { updateButtons(); });
    }
}

void VariableManagerWidget::save()
{
    const QString& file = QFileDialog::getSaveFileName(this, i18n("Save"), QString(),  QString());
    if (file.trimmed().isEmpty())
        return;

    auto* ext = dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));
    if (ext)
    {
        const QString& cmd = ext->saveVariables(file);
        emit runCommand(cmd);
    }
}

void VariableManagerWidget::load()
{
    const QString& file = QFileDialog::getOpenFileName(this, i18n("Load file"), QString(),  QString());
    if (file.trimmed().isEmpty())
        return;

    auto* ext = dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));
    if (ext)
    {
        const QString& cmd = ext->loadVariables(file);
        emit runCommand(cmd);
    }
}

void VariableManagerWidget::newVariable()
{
    QPointer<QDialog> dlg = new QDialog(this);
    dlg->setWindowTitle(i18n("Add New Variable"));
    dlg->setWindowIcon(QIcon::fromTheme(QLatin1String("document-new")));

    QWidget* widget = new QWidget(dlg);
    Ui::NewVariableDialogBase base;
    base.setupUi(widget);

    auto* mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);

    base.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton));
    base.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

    connect(base.buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()) );
    connect(base.buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()) );

    mainLayout->addWidget(widget);

    if (dlg->exec())
    {
        const QString& name = base.name->text();
        const QString& val = base.value->text();

        auto* ext = dynamic_cast<Cantor::VariableManagementExtension*>(m_session->backend()->extension(QLatin1String("VariableManagementExtension")));
        if (ext)
        {
            const QString& cmd = ext->addVariable(name, val);
            emit runCommand(cmd);
        }
    }

    delete dlg;
}

/*!
  toggles the menu for the filter/search options
*/
void VariableManagerWidget::toggleFilterOptionsMenu(bool checked) {
    if (checked) {
        QMenu menu;
        menu.addAction(m_caseSensitiveAction);
        menu.addAction(m_matchCompleteWordAction);
        connect(&menu, &QMenu::aboutToHide, m_bFilterOptions, &QPushButton::toggle);
        menu.exec(m_bFilterOptions->mapToGlobal(QPoint(0, m_bFilterOptions->height())));
    }
}

/*!
  called when the filter/search text was changed.
*/
void VariableManagerWidget::filterTextChanged(const QString& text) {
    auto sensitivity = m_caseSensitiveAction->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool matchCompleteWord = m_matchCompleteWordAction->isChecked();
    const auto* model = m_treeView->model();

    for (int i = 0; i < model->rowCount(); i++) {
        const auto& child = model->index(i, 0);
        const auto& name = model->data(child).toString();
        bool visible = true;
        if (text.isEmpty())
            visible = true;
        else if (matchCompleteWord)
            visible = name.startsWith(text, sensitivity);
        else
            visible = name.contains(text, sensitivity);

        m_treeView->setRowHidden(i, QModelIndex(), !visible);
    }
}

void VariableManagerWidget::updateButtons()
{
    bool enabled = (m_treeView->model()->rowCount() != 0);
    m_saveBtn->setEnabled(enabled);
    m_clearBtn->setEnabled(enabled);
}

void VariableManagerWidget::contextMenuEvent(QContextMenuEvent* event) {
    const auto& index  = m_treeView->currentIndex();
    if (!index.isValid())
        return;

    // initialize the actions if not done yet
    if (!m_copyNameAction)
    {
        auto* group = new QActionGroup(this);
        m_copyNameAction = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy Name"), group);
        m_copyValueAction = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy Value"), group);
        m_copyNameValueAction = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy Name and Value"), group);
        connect(group, &QActionGroup::triggered, this, &VariableManagerWidget::copy);

        //m_dataViewerAction = new QAction(QIcon::fromTheme(QLatin1String("document-preview")), i18n("Data Viewer"), this);
    }

    auto* menu = new QMenu(this);
    menu->addAction(m_copyNameAction);
    menu->addAction(m_copyValueAction);
    menu->addAction(m_copyNameValueAction);
    //menu->addSeparator();
    //menu->addAction(m_dataViewerAction);

    menu->exec(event->globalPos());
    delete menu;
}

void VariableManagerWidget::copy(const QAction* action) const
{
    const auto& items = m_treeView->selectionModel()->selectedIndexes();
    QString text;
    if (action == m_copyNameAction)
        text = items.at(0).data().toString();
    else if (action == m_copyValueAction)
    {
        text = items.at(1).data().toString();
        text = text.replace(QStringLiteral("; "), QStringLiteral("\n"));
    }
    else if (action == m_copyNameValueAction)
    {
        text = items.at(0).data().toString();
        text += QLatin1Char('\n') + items.at(1).data().toString();
        text = text.replace(QStringLiteral("; "), QStringLiteral("\n"));
    }

    QApplication::clipboard()->setText(text);
}
