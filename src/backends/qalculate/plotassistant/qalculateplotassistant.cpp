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

#include "qalculateplotassistant.h"

#include <kaction.h>
#include <kactioncollection.h>
#include "cantor_macros.h"

QalculatePlotAssistant::QalculatePlotAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args);

    m_dlg = 0;
}

QalculatePlotAssistant::~QalculatePlotAssistant()
{
}

void QalculatePlotAssistant::initActions()
{
    setXMLFile("cantor_qalculateplotassistant.rc");
    KAction* plot  = new KAction(i18n("Plot"), actionCollection());
    actionCollection()->addAction("qalculateplotassistant", plot);
    connect(plot, SIGNAL(triggered()), this, SIGNAL(requested()));
}

void QalculatePlotAssistant::initDialog(QWidget* parent)
{
    m_dlg = new KDialog(parent);
    QWidget *widget = new QWidget(m_dlg);
    m_base.setupUi(widget);
    m_dlg->setMainWidget(widget);

    connect(m_base.addButton, SIGNAL(clicked()), this, SLOT(addFunction()));
    connect(m_base.removeButton, SIGNAL(clicked()), this, SLOT(removeSelection()));
    connect(m_base.clearButton, SIGNAL(clicked()), this, SLOT(clearFunctions()));
    connect(m_base.functionTable, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(currentItemChanged(int, int, int, int)));
    connect(m_base.stepsButton, SIGNAL(toggled(bool)), this, SLOT(toggleStep()));
    connect(m_base.stepButton, SIGNAL(toggled(bool)), this, SLOT(toggleSteps()));
    m_base.inlineCheckBox->setChecked(QalculateSettings::inlinePlot());
    m_base.colorCheckBox->setChecked(QalculateSettings::coloredPlot());
    m_base.gridCheckBox->setChecked(QalculateSettings::plotGrid());
    m_base.borderCheckBox->setChecked(QalculateSettings::plotBorder());
    m_base.smoothingBox->setCurrentIndex(QalculateSettings::plotSmoothing());
    m_base.styleBox->setCurrentIndex(QalculateSettings::plotStyle());
    m_base.legendBox->setCurrentIndex(QalculateSettings::plotLegend());
    m_base.stepsEdit->setText(QString::number(QalculateSettings::plotSteps()));
    m_base.stepEdit->setDisabled(true);
}

QStringList QalculatePlotAssistant::run(QWidget* parent)
{
    if (!m_dlg)
	initDialog(parent);
	
    QStringList result;
    if (m_dlg->exec()) {
	if (m_base.functionTable->currentRow() >= 0)
	    saveRowInformation(m_base.functionTable->currentRow());
	result << plotCommand();
    }

    return result;
}


void QalculatePlotAssistant::addFunction()
{
    m_base.functionTable->insertRow(m_base.functionTable->rowCount());
    m_xVarList << "";
    m_styleList << QalculateSettings::STYLE_LINES;
    m_smoothingList << QalculateSettings::SMOOTHING_NONE;
    saveRowInformation(m_xVarList.size()-1);
}

void QalculatePlotAssistant::removeSelection()
{
    int r = m_base.functionTable->currentRow();
    if (r < 0)
	return;
    m_base.functionTable->removeRow(r);
    if (m_xVarList.size() > r) {
	m_xVarList.removeAt(r);
	m_styleList.removeAt(r);
	m_smoothingList.removeAt(r);
    }
}

void QalculatePlotAssistant::clearFunctions()
{
    m_xVarList.clear();
    m_styleList.clear();
    m_smoothingList.clear();
    while (m_base.functionTable->rowCount())
	m_base.functionTable->removeRow(0);
}

void QalculatePlotAssistant::toggleSteps()
{
    m_base.stepsButton->setChecked(!m_base.stepButton->isChecked());
}

void QalculatePlotAssistant::toggleStep()
{
    m_base.stepButton->setChecked(!m_base.stepsButton->isChecked());
}

void QalculatePlotAssistant::currentItemChanged(int newRow, int newColumn, int oldRow, int oldColumn)
{
    Q_UNUSED(newColumn);
    Q_UNUSED(oldColumn);

    if (oldRow >= 0)
	saveRowInformation(oldRow);
    if (newRow >= 0)
	loadRowInformation(newRow);
}

void QalculatePlotAssistant::saveRowInformation(int row)
{
    m_xVarList[row] = m_base.xVarEdit->text();
    switch(m_base.styleBox->currentIndex()) {
    case 0:
	m_styleList[row] = QalculateSettings::STYLE_LINES; break;
    case 1:
	m_styleList[row] = QalculateSettings::STYLE_POINTS; break;
    case 2:
	m_styleList[row] = QalculateSettings::STYLE_LINES_POINTS; break;
    case 3:
	m_styleList[row] = QalculateSettings::STYLE_BOXES; break;
    case 4:
	m_styleList[row] = QalculateSettings::STYLE_HISTOGRAM; break;
    case 5:
	m_styleList[row] = QalculateSettings::STYLE_STEPS; break;
    case 6:
	m_styleList[row] = QalculateSettings::STYLE_CANDLESTICKS; break;
    case 7:
	m_styleList[row] = QalculateSettings::STYLE_DOTS; break;
    }
    switch(m_base.smoothingBox->currentIndex()) {
    case 0:
	m_smoothingList[row] = QalculateSettings::SMOOTHING_NONE; break;
    case 1:
	m_smoothingList[row] = QalculateSettings::SMOOTHING_UNIQUE; break;
    case 2:
	m_smoothingList[row] = QalculateSettings::SMOOTHING_CSPLINES; break;
    case 3:
	m_smoothingList[row] = QalculateSettings::SMOOTHING_BEZIER; break;
    case 4:
	m_smoothingList[row] = QalculateSettings::SMOOTHING_SBEZIER; break;
    }
}

void QalculatePlotAssistant::loadRowInformation(int row)
{
    m_base.xVarEdit->setText(m_xVarList[row]);
    m_base.styleBox->setCurrentIndex(m_styleList[row]);
    m_base.smoothingBox->setCurrentIndex(m_smoothingList[row]);
}

QString QalculatePlotAssistant::plotCommand()
{
    QStringList boolList;
    boolList << "false" << "true";
    QString command = "plot";
    if (!m_base.plotTitleEdit->text().isEmpty())
	command += QString(" plottitle='%1'").arg(m_base.plotTitleEdit->text());
    if (!m_base.xLabelEdit->text().isEmpty())
	command += QString(" xlabel='%1'").arg(m_base.xLabelEdit->text());
    if (!m_base.yLabelEdit->text().isEmpty())
	command += QString(" ylabel='%1'").arg(m_base.yLabelEdit->text());
    if (m_base.legendBox->currentIndex() != QalculateSettings::plotLegend()) {
	QString legend;
	switch(m_base.legendBox->currentIndex()) {
	case QalculateSettings::LEGEND_NONE:
	    legend="none"; break;
	case QalculateSettings::LEGEND_TOP_LEFT:
	    legend="top_left"; break;
	case QalculateSettings::LEGEND_TOP_RIGHT:
	    legend="top_right"; break;
	case QalculateSettings::LEGEND_BOTTOM_LEFT:
	    legend="bottom_left"; break;
	case QalculateSettings::LEGEND_BOTTOM_RIGHT:
	    legend="bottom_right"; break;
	case QalculateSettings::LEGEND_BELOW:
	    legend="below"; break;
	case QalculateSettings::LEGEND_OUTSIDE:
	    legend="outside"; break;
	}
	command += QString(" legend=%1").arg(legend);
    }
    if (m_base.gridCheckBox->isChecked() != QalculateSettings::plotGrid())
	command += QString(" grid=%1").arg(boolList[m_base.gridCheckBox->isChecked()]);
    if (m_base.borderCheckBox->isChecked() != QalculateSettings::plotBorder())
	command += QString(" border=%1").arg(boolList[m_base.borderCheckBox->isChecked()]);
    if (m_base.colorCheckBox->isChecked() != QalculateSettings::coloredPlot())
	command += QString(" color=%1").arg(boolList[m_base.colorCheckBox->isChecked()]);
    if (m_base.inlineCheckBox->isChecked() != QalculateSettings::inlinePlot())
	command += QString(" inline=%1").arg(boolList[m_base.inlineCheckBox->isChecked()]);
    if (m_base.xLogCheckBox->isChecked())
	command += QString(" xlog=true xlogbase='%1'").arg(m_base.xLogEdit->text());
    if (m_base.yLogCheckBox->isChecked())
	command += QString(" ylog=true ylogbase='%1'").arg(m_base.yLogEdit->text());
    if (m_base.saveCheckBox->isChecked()) {
	QString filetype;
	switch (m_base.saveFileBox->currentIndex()) {
	case 0:
	    filetype = "auto"; break;
	case 1:
	    filetype = "png"; break;
	case 2:
	    filetype = "ps"; break;
	case 3:
	    filetype = "eps"; break;
	case 4:
	    filetype = "latex"; break;
	case 5:
	    filetype = "svg"; break;
	case 6:
	    filetype = "fig"; break;
	}
	command += QString(" filename='%1' filetype=%2").arg
	    (m_base.saveFileEdit->text(), filetype);
    }
    command += QString(" xmin='%1' xmax='%2'").arg
	(m_base.xMinEdit->text(), m_base.xMaxEdit->text());
    if (m_base.stepsButton->isChecked())
	command += QString(" steps='%1'").arg(m_base.stepsEdit->text());
    else
	command += QString(" step='%1'").arg(m_base.stepEdit->text());
    for (int i = 0; i < m_xVarList.size(); ++i) {
	if (i>0)
	    command += ',';
	command += QString(" title='%1' '%2' xvar='%3'").arg
	    (m_base.functionTable->item(i,0)->text(),
	     m_base.functionTable->item(i,1)->text(),
	     m_xVarList[i]);
	if (m_styleList[i] != QalculateSettings::plotStyle()) {
	    QString style;
	    switch(m_styleList[i]) {
	    case QalculateSettings::STYLE_LINES:
		style="lines"; break;
	    case QalculateSettings::STYLE_POINTS:
		style="points"; break;
	    case QalculateSettings::STYLE_LINES_POINTS:
		style="points_lines"; break;
	    case QalculateSettings::STYLE_BOXES:
		style="boxes"; break;
	    case QalculateSettings::STYLE_HISTOGRAM:
		style="histogram"; break;
	    case QalculateSettings::STYLE_STEPS:
		style="steps"; break;
	    case QalculateSettings::STYLE_CANDLESTICKS:
		style="candlesticks"; break;
	    case QalculateSettings::STYLE_DOTS:
		style="dots"; break;
	    }
	    command += QString(" style=%1").arg(style);
	}
	if (m_smoothingList[i] != QalculateSettings::plotSmoothing()) {
	    QString smoothing;
	    switch(m_smoothingList[i]) {
	    case QalculateSettings::SMOOTHING_NONE:
		smoothing="none"; break;
	    case QalculateSettings::SMOOTHING_UNIQUE:
		smoothing="monotonic"; break;
	    case QalculateSettings::SMOOTHING_CSPLINES:
		smoothing="csplines"; break;
	    case QalculateSettings::SMOOTHING_BEZIER:
		smoothing="bezier"; break;
	    case QalculateSettings::SMOOTHING_SBEZIER:
		smoothing="sbezier"; break;
	    }
	    command += QString(" smoothing=%1").arg(smoothing);
	}
    }
    return command;
}

K_EXPORT_CANTOR_PLUGIN(qalculateplotassistant, QalculatePlotAssistant)
