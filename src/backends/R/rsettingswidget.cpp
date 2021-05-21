/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
*/

#include "rsettingswidget.h"
#include <QGroupBox>
#include <KLineEdit>
#include <QFileDialog>
#include <KLocalizedString>
#include <QMouseEvent>

RSettingsWidget::RSettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
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
