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
    Copyright (C) 2010 Oleksiy Protas <elfy.ua@gmail.com>
    Copyright (C) 2020 Alexander Semke <alexander.semke@web.de>
 */

#include "rsettingswidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <KLocalizedString>

RSettingsWidget::RSettingsWidget(QWidget *parent, const QString& id) : BackendSettingsWidget(parent, id)
{
    setupUi(this);

    m_tabWidget = tabWidget;
    m_tabDocumentation = tabDocumentation;
    connect(tabWidget, &QTabWidget::currentChanged, this, &BackendSettingsWidget::tabChanged);

    kcfg_autorunScripts->lineEdit()->setReadOnly(true);
    kcfg_autorunScripts->lineEdit()->installEventFilter(this);
    kcfg_autorunScripts->lineEdit()->setToolTip(i18n("Double click to open file selection dialog"));
}

bool RSettingsWidget::eventFilter(QObject *obj, QEvent *event)
{
    /* Intercepting the doubleclick events of LineEdit */
    if (obj == kcfg_autorunScripts->lineEdit() && event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *ev=reinterpret_cast<QMouseEvent*>(event);
        if (ev->button() == Qt::LeftButton)
        {
            displayFileSelectionDialog();
            return false;
        }
    }
    return QObject::eventFilter(obj,event);
}

void RSettingsWidget::displayFileSelectionDialog()
{
    QString path=QFileDialog::getOpenFileName(this,kcfg_autorunScripts->lineEdit()->text(),QLatin1String("/home"),i18n("*.R *.r|R source files (*.R, *.r)"));
    if (path.length()>0)
        kcfg_autorunScripts->lineEdit()->setText(path);
}
