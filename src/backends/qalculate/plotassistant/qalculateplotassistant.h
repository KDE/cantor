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
    Copyright (C) 2011 Martin Kuettler <martinkuettler@gmail.com>
 */

#ifndef QALCULATE_PLOT_ASSISTANT_H
#define QALCULATE_PLOT_ASSISTANT_H

#include "assistant.h"
#include "settings.h"
#include "ui_qalculateplotdialog.h"
#include <kdialog.h>
#include <qlist.h>

class QalculatePlotAssistant : public Cantor::Assistant
{
    Q_OBJECT
private:
    KDialog* m_dlg;
    Ui::QalculatePlotAssistantBase m_base;
    QStringList m_xVarList;
    QList<QalculateSettings::PlotStyle> m_styleList;
    QList<QalculateSettings::SmoothingMode> m_smoothingList;

    QString plotCommand();
    void initDialog(QWidget* parent);
    void saveRowInformation(int);
    void loadRowInformation(int);
public slots:
    void addFunction();
    void removeSelection();
    void clearFunctions();
    void currentItemChanged(int, int, int, int);
    void toggleSteps();
    void toggleStep();

public:
    QalculatePlotAssistant(QObject* parent, QList<QVariant> args);
    ~QalculatePlotAssistant();

    void initActions();

    QStringList run(QWidget* parent);
};

#endif //QALCULATE_PLOT_ASSISTANT_H
