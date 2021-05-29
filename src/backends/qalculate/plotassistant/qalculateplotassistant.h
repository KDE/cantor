/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Martin Kuettler <martinkuettler@gmail.com>
*/

#ifndef QALCULATE_PLOT_ASSISTANT_H
#define QALCULATE_PLOT_ASSISTANT_H

#include "assistant.h"
#include "settings.h"
#include "ui_qalculateplotdialog.h"
#include <QDialog>
#include <QList>

class QalculatePlotAssistant : public Cantor::Assistant
{
    Q_OBJECT
private:
    QDialog* m_dlg;
    Ui::QalculatePlotAssistantBase m_base;
    QStringList m_xVarList;
    QList<QalculateSettings::PlotStyle> m_styleList;
    QList<QalculateSettings::SmoothingMode> m_smoothingList;

    QString plotCommand();
    void initDialog(QWidget* parent);
    void saveRowInformation(int);
    void loadRowInformation(int);
public Q_SLOTS:
    void addFunction();
    void removeSelection();
    void clearFunctions();
    void currentItemChanged(int, int, int, int);
    void toggleSteps();
    void toggleStep();

public:
    QalculatePlotAssistant(QObject* parent, QList<QVariant> args);
    ~QalculatePlotAssistant() override = default;

    void initActions() override;

    QStringList run(QWidget* parent) override;
};

#endif //QALCULATE_PLOT_ASSISTANT_H
