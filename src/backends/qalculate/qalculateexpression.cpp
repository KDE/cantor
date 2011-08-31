/*************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                             *
*  Copyright (C) 2011 by Matteo Agostinelli <agostinelli@gmail.com>                 *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include <config-cantorlib.h>

#include "textresult.h"
#include "helpresult.h"
#include "imageresult.h"
#include "epsresult.h"
#include "settings.h"

#include "qalculateexpression.h"
#include "qalculatesession.h"
#include "qalculatesyntaxhelpobject.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include <string>
// required for the plotting interface of Qalculator
#include <vector>

#include <KGlobal>
#include <KMessageBox>
#include <KColorScheme>
#include <KLocale>
#include <KDebug>

#include <QApplication>

QalculateExpression::QalculateExpression( QalculateSession* session )
    : Cantor::Expression(session)
{
    m_tempFile = 0;
}

QalculateExpression::~QalculateExpression()
{
    if (m_tempFile)
	delete m_tempFile;
}

void QalculateExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);
    m_message = "";

    if (command().isEmpty()) {
        return;
    }

    if (command().contains("help")) {
        QalculateSyntaxHelpObject* helper = new QalculateSyntaxHelpObject(command(), (QalculateSession*) session());
        setResult(new Cantor::HelpResult(helper->answer()));
        setStatus(Cantor::Expression::Done);
        return;
    }
    else if (command().trimmed().startsWith("plot") &&
	     (command().indexOf("plot")+4 == command().size() ||
	      command()[command().indexOf("plot")+4].isSpace())) {
        evaluatePlotCommand();
	return;
    }
    else if (command().trimmed().startsWith("saveVariables") &&
	     (command().indexOf("saveVariables")+13 == command().size() ||
	      command()[command().indexOf("saveVariables")+13].isSpace())) {
        evaluateSaveVariablesCommand();
	return;
    }
    else if (command().trimmed().startsWith("loadVariables") &&
	     (command().indexOf("loadVariables")+13 == command().size() ||
	      command()[command().indexOf("loadVariables")+13].isSpace())) {
        evaluateLoadVariablesCommand();
	return;
    }

    string expression = unlocalizeExpression(command());

    kDebug() << "EXPR: " << QString(expression.c_str());

    EvaluationOptions eo = evaluationOptions();

    MathStructure result = CALCULATOR->calculate(expression, eo);

    // update the answer variables
    static_cast<QalculateSession*>(session())->setLastResult(result);

    // error handling
    if (checkForCalculatorMessages() & (MSG_WARN | MSG_WARN))
	return;

    PrintOptions po;

    switch (QalculateSettings::fractionFormat()) {
    case 0:
        po.number_fraction_format = FRACTION_DECIMAL;
        break;
    case 1:
        po.number_fraction_format = FRACTION_DECIMAL_EXACT;
        break;
    case 2:
        po.number_fraction_format = FRACTION_FRACTIONAL;
        break;
    case 3:
        po.number_fraction_format = FRACTION_COMBINED;
        break;
    }
    po.indicate_infinite_series = QalculateSettings::indicateInfiniteSeries();
    po.use_all_prefixes = QalculateSettings::useAllPrefixes();
    po.negative_exponents = QalculateSettings::negativeExponents();

    po.lower_case_e = true;
    po.base = QalculateSettings::base();
    po.decimalpoint_sign = KGlobal::locale()->decimalSymbol().toLocal8Bit().data();

    switch (QalculateSettings::minExp()) {
    case 0:
        po.min_exp = EXP_NONE;
        break;
    case 1:
        po.min_exp = EXP_PURE;
        break;
    case 2:
        po.min_exp = EXP_SCIENTIFIC;
        break;
    case 3:
        po.min_exp = EXP_PRECISION;
        break;
    case 4:
        po.min_exp = EXP_BASE_3;
        break;
    }

    result.format(po);

    setResult(new Cantor::TextResult(result.print(po).c_str()));
    setStatus(Done);
}

void QalculateExpression::evaluateSaveVariablesCommand()
{
    QString argString = command().mid(command().indexOf("saveVariables")+13);
    argString = argString.trimmed();

    QString usage = i18n("Usage: saveVariables file");

    QString fileName = parseForFilename(argString, usage);
    if (fileName.isNull())
	return;
    
    // We want to save Temporary variables, but Qalculate does not.
    std::vector<Variable*> variables = CALCULATOR->variables;
    // If somebody saves his variables in Cantor_Internal_Temporary
    // he deserves unexpected behavior.
    std::string tmpCategory = "Temporary";
    std::string newCategory = "Cantor_Internal_Temporary";
    for (int i = 0; i < variables.size(); ++i) {
	if (variables[i]->category() == tmpCategory)
	    variables[i]->setCategory(newCategory);
    }

    int res = CALCULATOR->saveVariables(fileName.toLatin1().data());

    for (int i = 0; i < variables.size(); ++i) {
	if (variables[i]->category() == newCategory)
	    variables[i]->setCategory(tmpCategory);
    }


    if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
	return;
    }
    if (res < 0) {
	showMessage(i18n("Saving failed."), MESSAGE_ERROR);
	return;
    }

    setStatus(Done);
}

void QalculateExpression::evaluateLoadVariablesCommand()
{
    QString argString = command().mid(command().indexOf("loadVariables")+13);
    argString = argString.trimmed();

    QString usage = i18n("Usage: loadVariables file");

    QString fileName = parseForFilename(argString, usage);
    if (fileName.isNull())
	return;
    
    int res = CALCULATOR->loadDefinitions(fileName.toLatin1().data());
    if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
	return;
    }
    if (res < 0) {
	showMessage(i18n("Loading failed."), MESSAGE_ERROR);
	return;
    }

    // We have to store temporary variables in a different category
    // (see parseSaveVariablesCommand())
    std::vector<Variable*> variables = CALCULATOR->variables;
    std::string tmpCategory = "Temporary";
    std::string newCategory = "Cantor_Internal_Temporary";

    for (int i = 0; i < variables.size(); ++i) {
	if (variables[i]->category() == newCategory)
	    variables[i]->setCategory(tmpCategory);
    }

    setStatus(Done);
}

QString QalculateExpression::parseForFilename(QString argument, QString usage)
{
    if (argument.isEmpty()) {
	showMessage(usage, MESSAGE_ERROR);
	return QString();
    }

    QString fileName = "";
    QChar sep = '\0';
    int i = 0;
    if (argument[0] == '\'' || argument[0] == '"') {
	sep = argument[0];
	i = 1;
    }
    while (i < argument.size() && !argument[i].isSpace() && 
	   argument[i] != sep) {
	if (argument[i] == '\\' && i < argument.size()-1)
	    ++i;
	fileName += argument[i];
	++i;
    }

    if (sep != '\0' && i == argument.size()) {
	showMessage(QString(i18n("missing %1").arg(sep)), MESSAGE_ERROR);
	return QString();
    }
    
    if (i < argument.size() - 1) {
	showMessage(usage, MESSAGE_ERROR);
	return QString();
    }

    return fileName;
}

void QalculateExpression::evaluatePlotCommand()
{
    QString argString = command().mid(command().indexOf("plot")+4);
    argString = unlocalizeExpression(argString).c_str();
    argString = argString.trimmed();

    QList<QStringList> argumentsList;
    QStringList arguments;

    // For error handling
    KColorScheme scheme(QApplication::palette().currentColorGroup());
    const QString errorColor = scheme.foreground(KColorScheme::NegativeText).color().name();
    const QString warningColor = scheme.foreground(KColorScheme::NeutralText).color().name();
    const QString msgFormat("<font color=\"%1\">%2: %3</font><br>\n");

    if (!CALCULATOR->canPlot()) {
	showMessage(i18n("Qalculate reports it cannot print. Is gnuplot installed?"), MESSAGE_ERROR);
	return;
    }

    // Split argString into the arguments
    int i=0;
    int j=0;
    QString arg = "";
    while (i < argString.size()) {
        if (argString[i] == '"' || argString[i] == '\'') {
	    ++j;
	    while(j < argString.size() && argString[j] != argString[i]) {
	        if (argString[j] == '\\') {
		    ++j;
		    if (j == argString.size())
			continue; // just ignore trailing backslashes
		}
		arg += argString[j];
	        ++j;
	    }
	    if (j == argString.size()) {
		showMessage(QString(i18n("missing %1")).arg(argString[i]), MESSAGE_ERROR);
		return;
	    }
	    ++j;
        } else if (argString[i] == ',') {
  	    argumentsList.append(arguments);
	    arguments.clear();
	    ++j;
	} else {
	    while(j < argString.size() && !argString[j].isSpace() && 
		  argString[j] != '=' && argString[j] != ',') {
	        if (argString[j] == '\\') {
		    ++j;
		    if (j == argString.size())
			continue; // just ignore trailing backslashes
		}
		arg += argString[j];
		++j;
	    }
	}
	if (argString[j] == '=') {
	    // Parse things like title="..." as one argument
	    arg += '=';
	    i = ++j;
	    continue;
	}
	if (!arg.isEmpty()) {
	    arguments << arg;
	    arg = "";
	}
	while (j < argString.size() && argString[j].isSpace())
	    ++j;
	i = j;
    }
    argumentsList.append(arguments);

    // Parse the arguments and compute the points to be plotted 
    std::vector<MathStructure> y_vectors;
    std::vector<MathStructure> x_vectors;
    std::vector<PlotDataParameters*> plotDataParameterList;
    PlotParameters plotParameters;
    EvaluationOptions eo = evaluationOptions();
    /// temporary
    plotParameters.title = "";
    plotParameters.y_label = "";
    plotParameters.x_label = "";
    plotParameters.filename = "";
    plotParameters.filetype = PLOT_FILETYPE_AUTO;
    plotParameters.color = QalculateSettings::coloredPlot();
    plotParameters.auto_y_min = true;
    plotParameters.auto_x_min = true;
    plotParameters.auto_x_max = true;
    plotParameters.auto_y_max = true;
    plotParameters.y_log = false;
    plotParameters.x_log = false;
    plotParameters.grid = QalculateSettings::plotGrid();
    plotParameters.linewidth = QalculateSettings::plotLineWidth();
    plotParameters.show_all_borders = QalculateSettings::plotBorder();
    switch (QalculateSettings::plotLegend()) {
    case QalculateSettings::LEGEND_NONE:
	plotParameters.legend_placement = PLOT_LEGEND_NONE;
	break;
    case QalculateSettings::LEGEND_TOP_LEFT:
	plotParameters.legend_placement = PLOT_LEGEND_TOP_LEFT;
	break;
    case QalculateSettings::LEGEND_TOP_RIGHT:
	plotParameters.legend_placement = PLOT_LEGEND_TOP_RIGHT;
	break;
    case QalculateSettings::LEGEND_BOTTOM_LEFT:
	plotParameters.legend_placement = PLOT_LEGEND_BOTTOM_LEFT;
	break;
    case QalculateSettings::LEGEND_BOTTOM_RIGHT:
	plotParameters.legend_placement = PLOT_LEGEND_BOTTOM_RIGHT;
	break;
    case QalculateSettings::LEGEND_BELOW:
	plotParameters.legend_placement = PLOT_LEGEND_BELOW;
	break;
    case QalculateSettings::LEGEND_OUTSIDE:
	plotParameters.legend_placement = PLOT_LEGEND_OUTSIDE;
	break;
    }
    bool plotInline = QalculateSettings::inlinePlot();
    MathStructure xMin;
    MathStructure xMax;
    xMin.setUndefined();
    xMax.setUndefined();
    MathStructure stepLength;
    stepLength.setUndefined();
    int steps = QalculateSettings::plotSteps();
    
    QString mustBeNumber = i18n("%1 must be a number.");
    QString mustBeInteger = i18n("%1 must be a integer.");
    QString mustBeBoolean = i18n("%1 must be a boolean.");
    QString invalidOption = i18n("invalid option for %1: %2");

    for (int i = 0; i < argumentsList.size(); ++i) {
	std::string xVariable = "x";
	PlotDataParameters* plotDataParams = new PlotDataParameters;
	plotDataParameterList.push_back(plotDataParams);
	plotDataParams->title = "";
	switch(QalculateSettings::plotSmoothing()) {
	case QalculateSettings::SMOOTHING_NONE:
	    plotDataParams->smoothing = PLOT_SMOOTHING_NONE;
	    break;
	case QalculateSettings::SMOOTHING_UNIQUE:
	    plotDataParams->smoothing = PLOT_SMOOTHING_UNIQUE;
	    break;
	case QalculateSettings::SMOOTHING_CSPLINES:
	    plotDataParams->smoothing = PLOT_SMOOTHING_CSPLINES;
	    break;
	case QalculateSettings::SMOOTHING_BEZIER:
	    plotDataParams->smoothing = PLOT_SMOOTHING_BEZIER;
	    break;
	case QalculateSettings::SMOOTHING_SBEZIER:
	    plotDataParams->smoothing = PLOT_SMOOTHING_SBEZIER;
	    break;
	}
	switch(QalculateSettings::plotStyle()) {
	case QalculateSettings::STYLE_LINES:
	    plotDataParams->style = PLOT_STYLE_LINES;
	    break;
	case QalculateSettings::STYLE_POINTS:
	    plotDataParams->style = PLOT_STYLE_POINTS;
	    break;
	case QalculateSettings::STYLE_LINES_POINTS:
	    plotDataParams->style = PLOT_STYLE_POINTS_LINES;
	    break;
	case QalculateSettings::STYLE_BOXES:
	    plotDataParams->style = PLOT_STYLE_BOXES;
	    break;
	case QalculateSettings::STYLE_HISTOGRAM:
	    plotDataParams->style = PLOT_STYLE_HISTOGRAM;
	    break;
	case QalculateSettings::STYLE_STEPS:
	    plotDataParams->style = PLOT_STYLE_STEPS;
	    break;
	case QalculateSettings::STYLE_CANDLESTICKS:
	    plotDataParams->style = PLOT_STYLE_CANDLESTICKS;
	    break;
	case QalculateSettings::STYLE_DOTS:
	    plotDataParams->style = PLOT_STYLE_DOTS;
	    break;
	}
	plotDataParams->yaxis2 = false;
	plotDataParams->xaxis2 = false;
	arguments = argumentsList[i];
	std::string expression;
	int lastExpressionEntry = -1;
	for (int j = 0; j < arguments.size(); ++j) {
	    QString argument = arguments[j];
	    // PlotParameters
	    if (argument.startsWith("plottitle="))
		plotParameters.title = argument.mid(10).toLatin1().data();
	    else if (argument.startsWith("ylabel="))
		plotParameters.y_label = argument.mid(7).toLatin1().data();
	    else if (argument.startsWith("xlabel="))
		plotParameters.x_label = argument.mid(7).toLatin1().data();
	    else if (argument.startsWith("filename="))
		plotParameters.filename = argument.mid(9).toLatin1().data();
	    else if (argument.startsWith("filetype=")) {
		QString option = argument.mid(9);
		if (option == "auto")
		    plotParameters.filetype = PLOT_FILETYPE_AUTO;
		else if (option == "png")
		    plotParameters.filetype = PLOT_FILETYPE_PNG;
		else if (option == "ps")
		    plotParameters.filetype = PLOT_FILETYPE_PS;
		else if (option == "eps")
		    plotParameters.filetype = PLOT_FILETYPE_EPS;
		else if (option == "latex")
		    plotParameters.filetype = PLOT_FILETYPE_LATEX;
		else if (option == "svg")
		    plotParameters.filetype = PLOT_FILETYPE_SVG;
		else if (option == "fig")
		    plotParameters.filetype = PLOT_FILETYPE_FIG;
		else {
		    QString msg = invalidOption.arg("filetype", option);
		    showMessage(msg, MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("font="))
		plotParameters.font = argument.mid(5).toLatin1().data();
	    else if (argument.startsWith("color=")) {
		bool ok;
		plotParameters.color = stringToBool(argument.mid(6), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("color"),
				MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("ylog=")) {
		bool ok;
		plotParameters.y_log = stringToBool(argument.mid(5), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("ylog"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("xlog=")) {
		bool ok;
		plotParameters.x_log = stringToBool(argument.mid(5), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("xlog"), MESSAGE_ERROR);
		    return;
		}
	    }
	    else if (argument.startsWith("ylogbase=")) {
		MathStructure ylogStr = CALCULATOR->calculate(argument.mid(9).toLatin1().data(), eo);
		if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
		if (ylogStr.isNumber()) {
		    Number ylogNum = ylogStr.number();
		    plotParameters.y_log_base = ylogNum.floatValue();
		} else {
		    showMessage(mustBeNumber.arg("ylogbase"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("xlogbase=")) {
		MathStructure xlogStr = CALCULATOR->calculate(argument.mid(9).toLatin1().data(), eo);
		if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
		if (xlogStr.isNumber()) {
		    Number xlogNum = xlogStr.number();
		    plotParameters.x_log_base = xlogNum.floatValue();
		} else {
		    showMessage(mustBeNumber.arg("xlogbase"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("grid=")) {
		bool ok;
		plotParameters.grid = stringToBool(argument.mid(5), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("grid"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("linewidth=")) {
		MathStructure lineWidthStr = CALCULATOR->calculate(argument.mid(10).toLatin1().data(), eo);
		Number lineWidthNum;
		if (lineWidthStr.isNumber() && lineWidthStr.number().isInteger()) {
		    lineWidthNum = lineWidthStr.number();
		    plotParameters.linewidth = lineWidthNum.intValue();
		} else {
		    showMessage(mustBeInteger.arg("linewidth"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("border=")) {
		bool ok;
		plotParameters.show_all_borders = stringToBool(argument.mid(7), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("border"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("legend=")) {
		QString option = argument.mid(7);
		if (option == "none")
		    plotParameters.legend_placement = PLOT_LEGEND_NONE;
		else if (option == "top_left")
		    plotParameters.legend_placement = PLOT_LEGEND_TOP_LEFT;
		else if (option == "top_right")
		    plotParameters.legend_placement = PLOT_LEGEND_TOP_RIGHT;
		else if (option == "bottom_left")
		    plotParameters.legend_placement = PLOT_LEGEND_BOTTOM_LEFT;
		else if (option == "bottom_right")
		    plotParameters.legend_placement = PLOT_LEGEND_BOTTOM_RIGHT;
		else if (option == "below")
		    plotParameters.legend_placement = PLOT_LEGEND_BELOW;
		else if (option == "outside")
		    plotParameters.legend_placement = PLOT_LEGEND_OUTSIDE;
		else {
		    QString msg = invalidOption.arg("legend", option);
		    showMessage(msg, MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    // PlotDataParameters
	    else if (argument.startsWith("title=")) {
		plotDataParams->title = argument.mid(6).toLatin1().data();
	    }
	    else if (argument.startsWith("smoothing=")) {
		QString option = argument.mid(10);
		if (option == "none")
		    plotDataParams->smoothing = PLOT_SMOOTHING_NONE;
		else if (option == "monotonic")
		    plotDataParams->smoothing = PLOT_SMOOTHING_UNIQUE;
		else if (option == "csplines")
		    plotDataParams->smoothing = PLOT_SMOOTHING_CSPLINES;
		else if (option == "bezier")
		    plotDataParams->smoothing = PLOT_SMOOTHING_BEZIER;
		else if (option == "sbezier")
		    plotDataParams->smoothing = PLOT_SMOOTHING_SBEZIER;
		else {
		    QString msg = invalidOption.arg("smoothing", option);
		    showMessage(msg, MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    } else if (argument.startsWith("style=")) {
		QString option = argument.mid(6);
		if (option == "lines")
		    plotDataParams->style = PLOT_STYLE_LINES;
		else if (option == "points")
		    plotDataParams->style = PLOT_STYLE_POINTS;
		else if (option == "points_lines")
		    plotDataParams->style = PLOT_STYLE_POINTS_LINES;
		else if (option == "boxes")
		    plotDataParams->style = PLOT_STYLE_BOXES;
		else if (option == "histogram")
		    plotDataParams->style = PLOT_STYLE_HISTOGRAM;
		else if (option == "steps")
		    plotDataParams->style = PLOT_STYLE_STEPS;
		else if (option == "candlesticks")
		    plotDataParams->style = PLOT_STYLE_CANDLESTICKS;
		else if (option == "dots")
		    plotDataParams->style = PLOT_STYLE_DOTS;
		else {
		    QString msg = invalidOption.arg("style", option);
		    showMessage(msg, MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    } else if (argument.startsWith("xaxis2=")) {
		bool ok;
		plotDataParams->xaxis2 = stringToBool(argument.mid(7), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("xaxis2"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    } else if (argument.startsWith("yaxis2=")) {
		bool ok;
		plotDataParams->yaxis2 = stringToBool(argument.mid(7), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("yaxis2"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    // inline, xmin, xmax, step, steps, xvar
	    // Custom options
	    else if (argument.startsWith("inline=")) {
		bool ok;
		plotInline = stringToBool(argument.mid(7), &ok);
		if (!ok) {
		    showMessage(mustBeBoolean.arg("inline"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("xmin=")) {
		xMin = CALCULATOR->calculate(argument.mid(5).toLatin1().data(), eo);
		if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("xmax=")) {
		xMax = CALCULATOR->calculate(argument.mid(5).toLatin1().data(), eo);
		if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("step=")) {
		stepLength = CALCULATOR->calculate(argument.mid(5).toLatin1().data(), eo);
		if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
		steps = -1;
	    }
	    else if (argument.startsWith("steps=")) {
		MathStructure stepsStr = CALCULATOR->calculate(argument.mid(6).toLatin1().data(), eo);
		if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
		Number stepsNum;
		if (stepsStr.isNumber() && stepsStr.number().isInteger()) {
		    stepsNum = stepsStr.number();
		    steps = stepsNum.intValue();
		    stepLength.setUndefined();
		} else {
		    showMessage(mustBeInteger.arg("steps"), MESSAGE_ERROR);
		    deletePlotDataParameters(plotDataParameterList);
		    return;
		}
	    }
	    else if (argument.startsWith("xvar=")) {
		xVariable = argument.mid(5).toLatin1().data();
	    }
	    else if (expression.empty()) {
		expression = argument.toLatin1().data();
		lastExpressionEntry = j;
	    }
	    else if (lastExpressionEntry == j-1) {
		expression += " ";
		expression += argument.toLatin1().data();
		lastExpressionEntry = j;
	    }
	    else {
		QString msg = i18n("found multiple expressions in one plot command (%1 and %2).");
		msg = msg.arg(QString(expression.c_str()), argument);
		showMessage(msg, MESSAGE_ERROR);
		deletePlotDataParameters(plotDataParameterList);
		return;
	    }
	}
	if (expression.empty())
	    continue;
	if (xMin.isUndefined()) {
	    if (!plotParameters.auto_x_min)
		xMin = plotParameters.x_min;
	    else
		xMin = 0.0;
	}
	if (xMax.isUndefined()) {
	    if (!plotParameters.auto_x_max)
		xMax = plotParameters.x_max;
	    else
		xMax = 10.0;
	}
	if (plotDataParams->title.empty())
	    plotDataParams->title = expression;
	MathStructure x_vec, y_vec;
	x_vec.clearVector();
	if (!stepLength.isUndefined())
	    y_vec = CALCULATOR->expressionToPlotVector(expression, xMin, xMax, stepLength, &x_vec, xVariable, eo.parse_options);
	else
	    y_vec = CALCULATOR->expressionToPlotVector(expression, xMin, xMax, steps, &x_vec, xVariable, eo.parse_options);
	if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
	    deletePlotDataParameters(plotDataParameterList);
	    return;
	}

	x_vectors.push_back(x_vec);
	y_vectors.push_back(y_vec);

	//PrintOptions po;
	//y_vec.format(po);
	
	//setResult(new Cantor::TextResult(y_vec.print(po).c_str()));
	//setStatus(Done);
	//deletePlotDataParameters(plotDataParameterList);
	//return;
    }

    if (plotInline && plotParameters.filename.empty()) {
	// TODO: get a temporary file name here
	if (!m_tempFile) {
	    m_tempFile = new KTemporaryFile();
#ifdef WITH_EPS
	    m_tempFile->setSuffix(".eps");
#else
	    m_tempFile->setSuffix(".png");
#endif
	    m_tempFile->open();
	}
	plotParameters.filename = m_tempFile->fileName().toLatin1().data();
	plotParameters.filetype = PLOT_FILETYPE_AUTO;
    }

    CALCULATOR->plotVectors(&plotParameters, y_vectors, x_vectors, 
			    plotDataParameterList);
    if (checkForCalculatorMessages() & (MSG_WARN|MSG_ERR)) {
	deletePlotDataParameters(plotDataParameterList);
	return;
    }

    deletePlotDataParameters(plotDataParameterList);

    if (plotInline) {
#ifdef WITH_EPS
	size_t p = plotParameters.filename.size();
	if (plotParameters.filetype == PLOT_FILETYPE_EPS ||
	    plotParameters.filetype == PLOT_FILETYPE_PS  ||
	    (plotParameters.filetype == PLOT_FILETYPE_AUTO && p >= 4 &&
	     plotParameters.filename.substr(p-4,4) == ".eps") ||
	    (plotParameters.filetype == PLOT_FILETYPE_AUTO && p >= 3 &&
	     plotParameters.filename.substr(p-3,3) == ".ps")) 
	    setResult(new Cantor::EpsResult(KUrl(plotParameters.filename.c_str())));
	else
	    setResult(new Cantor::ImageResult(KUrl(plotParameters.filename.c_str())));
#else
	setResult(new Cantor::ImageResult(KUrl(plotParameters.filename.c_str())));
#endif
	setStatus(Cantor::Expression::Done);
    }

}

void QalculateExpression::showMessage(QString msg, MessageType mtype)
{
    KColorScheme scheme(QApplication::palette().currentColorGroup());
    const QString errorColor = scheme.foreground(KColorScheme::NegativeText).color().name();
    const QString warningColor = scheme.foreground(KColorScheme::NeutralText).color().name();
    const QString msgFormat("<font color=\"%1\">%2: %3</font><br>\n");
    if(mtype == MESSAGE_ERROR || mtype == MESSAGE_WARNING) {
	msg.replace("&", "&amp;");
	msg.replace(">", "&gt;");
	msg.replace("<", "&lt;");

	if (mtype == MESSAGE_ERROR) {
	    msg = msgFormat.arg(errorColor, i18n("ERROR"), msg.toLatin1().data());
	} else {
	    msg = msgFormat.arg(errorColor, i18n("WARNING"), msg.toLatin1().data());
	}
	setErrorMessage(msg);
	setStatus(Error);
    } else {
	KMessageBox::information(QApplication::activeWindow(), msg);
    }
}

EvaluationOptions QalculateExpression::evaluationOptions()
{
    EvaluationOptions eo;

    eo.auto_post_conversion = QalculateSettings::postConversion() ? POST_CONVERSION_BEST : POST_CONVERSION_NONE;
    eo.keep_zero_units = false;

    eo.parse_options = parseOptions();

    switch (QalculateSettings::structuring()) {
        case 0:
            eo.structuring = STRUCTURING_NONE;
            break;
        case 1:
            eo.structuring = STRUCTURING_SIMPLIFY;
            break;
        case 2:
            eo.structuring = STRUCTURING_FACTORIZE;
            break;
    }
    
    return eo;
}

ParseOptions QalculateExpression::parseOptions()
{
    ParseOptions po;
    switch (QalculateSettings::angleUnit()) {
        case 0:
            po.angle_unit = ANGLE_UNIT_NONE;
            break;
        case 1:
            po.angle_unit = ANGLE_UNIT_RADIANS;
            break;
        case 2:
            po.angle_unit = ANGLE_UNIT_DEGREES;
            break;
        case 3:
            po.angle_unit = ANGLE_UNIT_GRADIANS;
            break;
    }

    po.base = QalculateSettings::base();

    return po;
}

void QalculateExpression::deletePlotDataParameters
    (const std::vector<PlotDataParameters*>& plotDataParameterList)
{
    for(int i = 0; i < plotDataParameterList.size(); ++i)
	delete plotDataParameterList[i];
}

bool QalculateExpression::stringToBool(const QString &string, bool *ok)
{
    if (string == "true" || string == "1") {
	*ok = true;
	return true;
    } else if (string == "false" || string == "0") {
	*ok = true;
	return false;
    } else {
	*ok = false;
	return false;
    }
}

int QalculateExpression::checkForCalculatorMessages()
{
    // error handling, most of it copied from qalculate-kde
    int msgType = MSG_NONE;
    if ( CALCULATOR->message() ) {
        QString msg;
        KColorScheme scheme(QApplication::palette().currentColorGroup());
        const QString errorColor = scheme.foreground(KColorScheme::NegativeText).color().name();
        const QString warningColor = scheme.foreground(KColorScheme::NeutralText).color().name();
        const QString msgFormat("<font color=\"%1\">%2: %3</font><br>\n");
        MessageType mtype;
        while(true) {
            mtype = CALCULATOR->message()->type();
	    switch(mtype) {
	    case MESSAGE_INFORMATION:
		msgType |= MSG_INFO; break;
	    case MESSAGE_WARNING:
		msgType |= MSG_WARN; break;
	    case MESSAGE_ERROR:
		msgType |= MSG_ERR;  break;
	    }
            if(mtype == MESSAGE_ERROR || mtype == MESSAGE_WARNING) {
                QString text = CALCULATOR->message()->message().c_str();
                text.replace("&", "&amp;");
                text.replace(">", "&gt;");
                text.replace("<", "&lt;");

                if (mtype == MESSAGE_ERROR) {
                    msg.append(msgFormat.arg(errorColor, i18n("ERROR"), text));
                } else {
                    msg.append(msgFormat.arg(errorColor, i18n("WARNING"), text));
                }
            } else {
                KMessageBox::information(QApplication::activeWindow(), CALCULATOR->message()->message().c_str());
            }
            if(!CALCULATOR->nextMessage()) break;
        }
        if ( !msg.isEmpty() ) {
	    m_message += msg;
            setErrorMessage(m_message);
            setStatus(Error);
        }
    }
    return msgType;
}

std::string QalculateExpression::unlocalizeExpression(QString expr)
{
    // copy'n'pasted from qalculate plasma applet

    return CALCULATOR->unlocalizeExpression(
             expr.replace(QChar(0xA3), "GBP")
                 .replace(QChar(0xA5), "JPY")
                 .replace("$", "USD")
                 .replace(QChar(0x20AC), "EUR")
                 .toLatin1().data()
           );
}
