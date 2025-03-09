/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
    SPDX-FileCopyrightText: 2020-2025 Alexander Semke <alexander.semke@web.de>
*/

#include "rsettingswidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QTimer>
#include <KLocalizedString>

RSettingsWidget::RSettingsWidget(QWidget *parent, const QString& id) : BackendSettingsWidget(parent, id)
{
    setupUi(this);

    m_tabWidget = tabWidget;
#ifdef HAVE_EMBEDDED_DOCUMENTATION
    m_tabDocumentation = tabDocumentation;
#else
    tabWidget->removeTab(2);
#endif
    connect(tabWidget, &QTabWidget::currentChanged, this, &BackendSettingsWidget::tabChanged);
    connect(kcfg_integratePlots, &QCheckBox::clicked, this, &RSettingsWidget::integratePlotsChanged);

    kcfg_autorunScripts->lineEdit()->setReadOnly(true);
    kcfg_autorunScripts->lineEdit()->installEventFilter(this);
    kcfg_autorunScripts->lineEdit()->setToolTip(i18n("Double click to open file selection dialog"));

    kcfg_inlinePlotFormat->setItemIcon(0, QIcon::fromTheme(QLatin1String("application-pdf")));
    kcfg_inlinePlotFormat->setItemIcon(1, QIcon::fromTheme(QLatin1String("image-svg+xml")));
    kcfg_inlinePlotFormat->setItemIcon(2, QIcon::fromTheme(QLatin1String("image-png")));

    // return to the event loop to show the settings
    // and then call the slot to update the state of the widgets
    QTimer::singleShot(0, this, [=]() {
        integratePlotsChanged(kcfg_integratePlots->isChecked());
    });
}

void RSettingsWidget::integratePlotsChanged(bool state)
{
    lPlotWidth->setEnabled(state);
    kcfg_plotWidth->setEnabled(state);
    lPlotHeight->setEnabled(state);
    kcfg_plotHeight->setEnabled(state);
    kcfg_inlinePlotFormat->setEnabled(state);
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
