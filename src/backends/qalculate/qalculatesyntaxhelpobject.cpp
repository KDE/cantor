/*************************************************************************************
 *  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                               *
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

#include "settings.h"
#include "qalculatesyntaxhelpobject.h"
#include "qalculatesession.h"
#include "settings.h"

#include <KLocale>
#include <KDebug>

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

QalculateSyntaxHelpObject::QalculateSyntaxHelpObject(const QString& command, QalculateSession* session)
    : SyntaxHelpObject(command, session), m_answer(QString())
{
}

void QalculateSyntaxHelpObject::fetchInformation()
{
    std::string cmd = command().remove("help").simplified().toLatin1().data();
    kDebug() << "HELP CALLED FOR:" << QString(cmd.c_str());

    if (cmd == "plot") {
	setPlotInformation();
	return;
    }
    if (cmd == "saveVariables") {
	setSaveVariablesInformation();
	return;
    }
    if (cmd == "loadVariables") {
	setLoadVariablesInformation();
	return;
    }

    ExpressionItem *item = CALCULATOR->getActiveExpressionItem(cmd);

    if (!item) {
        m_answer = i18n("No function, variable or unit with specified name exist.");
        return;
    }

    switch(item->type()) {
    case TYPE_FUNCTION:
        MathFunction *f = (MathFunction*) item;
        QString title = i18n("Function: %1").arg(item->title().c_str());
        const ExpressionName *ename = &f->preferredName(false);
        int iargs = f->maxargs();
        if(iargs < 0) {
            iargs = f->minargs() + 1;
        }
        QString str,str2,syntax;
        str += ename->name.c_str();
        str += "(";
        if(iargs != 0) {
            Argument *arg;
            Argument default_arg;
            for(int i2 = 1; i2 <= iargs; i2++) {
                if(i2 > f->minargs()) {
                    str += "[";
                }
                if(i2 > 1) {
                    str += QString(CALCULATOR->getComma().c_str());
                    str += " ";
                }
                arg = f->getArgumentDefinition(i2);
                if(arg && !arg->name().empty()) {
                    str2 = arg->name().c_str();
                } else {
                    str2 = "argument";
                    str2 += " ";
                    str2 += QString::number(i2);
                }
                str += str2;
                if(i2 > f->minargs()) {
                    str += "]";
                }
            }
            if(f->maxargs() < 0) {
                str += CALCULATOR->getComma().c_str();
                str += " ...";
            }
	}
	str += ")";
	syntax = QString("<p>%1</p>").arg(str);

	QString arguments = "";
        if(iargs != 0) {
            Argument *arg;
            Argument default_arg;
            for(int i2 = 1; i2 <= iargs; i2++) {
                arg = f->getArgumentDefinition(i2);
                if(arg && !arg->name().empty()) {
                    str = arg->name().c_str();
                } else {
                    str = QString::number(i2);
                }
                str += ": ";
                if(arg) {
                    str2 = arg->printlong().c_str();
                } else {
                    str2 = default_arg.printlong().c_str();
                }
                if(i2 > f->minargs()) {
                    str2 += " (";
                    //optional argument, in description
                    str2 += "optional";
                    if(!f->getDefaultValue(i2).empty()) {
                        str2 += ", ";
                        //argument default, in description
                        str2 += "default: ";
                        str2 += f->getDefaultValue(i2).c_str();
                    }
                    str2 += ")";
                }
                str += str2;
		arguments += QString("<p>%1</p>").arg(str);
            }
        }

        QString desc = QString("<p>%1</p>").arg(item->description().c_str());

        m_answer = title + desc + syntax + arguments;
    }
}

void QalculateSyntaxHelpObject::setPlotInformation()
{
    QString title = "<p>" + i18n("Plotting interface") + "</p>";
    QString desc = "<p>" + i18n("Plots one or more functions either inline or in a seperate window.") + "</p>";
    QString expression = i18n("expression");
    QString option = i18n("option");
    QString value = i18n("value");
    QString syntax = QString("<p>plot %1 [%2=%3 ...] [, %4 [%5=%6 ...]] ...</p>");
    syntax = syntax.arg(expression, option, value, expression, option, value);
    
    QString integer = i18n("integer");
    QString boolean = i18n("boolean");
    QString number = i18n("number");
    QString defaultValue = i18n("default: %1");
    QString noDefault = "";
    QString optionFormat2 = "<p>%1: %2</p>";
    QString optionFormat3 = "<p>%1: %2 (%3)</p>";
    QString optionFormat4 = "<p>%1: %2 (%3, %4)</p>";

    QStringList boolList;
    boolList << "false" << "true";

    QString legendDefault;
    QString styleDefault;
    QString smoothingDefault;
    switch (QalculateSettings::plotLegend()) {
    case QalculateSettings::LEGEND_NONE:
	legendDefault = "none"; break;
    case QalculateSettings::LEGEND_TOP_LEFT:
	legendDefault = "top_left"; break;
    case QalculateSettings::LEGEND_TOP_RIGHT:
	legendDefault = "top_right"; break;
    case QalculateSettings::LEGEND_BOTTOM_LEFT:
	legendDefault = "bottom_left"; break;
    case QalculateSettings::LEGEND_BOTTOM_RIGHT:
	legendDefault = "bottom_right"; break;
    case QalculateSettings::LEGEND_BELOW:
	legendDefault = "below"; break;
    case QalculateSettings::LEGEND_OUTSIDE:
	legendDefault = "outside"; break;
    }
    switch(QalculateSettings::plotSmoothing()) {
    case QalculateSettings::SMOOTHING_NONE:
	smoothingDefault = "none";	break;
    case QalculateSettings::SMOOTHING_UNIQUE:
	smoothingDefault = "monotonic"; break;
    case QalculateSettings::SMOOTHING_CSPLINES:
	smoothingDefault = "csplines"; break;
    case QalculateSettings::SMOOTHING_BEZIER:
	smoothingDefault = "bezier"; break;
    case QalculateSettings::SMOOTHING_SBEZIER:
	smoothingDefault = "sbezier"; break;
    }
    switch(QalculateSettings::plotStyle()) {
    case QalculateSettings::STYLE_LINES:
	styleDefault = "lines"; break;
    case QalculateSettings::STYLE_POINTS:
	styleDefault = "points"; break;
    case QalculateSettings::STYLE_LINES_POINTS:
	styleDefault = "points_lines"; break;
    case QalculateSettings::STYLE_BOXES:
	styleDefault = "boxes"; break;
    case QalculateSettings::STYLE_HISTOGRAM:
	styleDefault = "histogram"; break;
    case QalculateSettings::STYLE_STEPS:
	styleDefault = "steps"; break;
    case QalculateSettings::STYLE_CANDLESTICKS:
	styleDefault = "candlesticks"; break;
    case QalculateSettings::STYLE_DOTS:
	styleDefault = "dots"; break;
    }

    QString arguments = "";
    arguments += optionFormat3.arg("title", i18n("The function's name"),
				   defaultValue.arg("expression"));
    arguments += optionFormat2.arg("plottitle", i18n("Title label"));
    arguments += optionFormat2.arg("xlabel", i18n("x-axis label"));
    arguments += optionFormat2.arg("ylabel", i18n("y-axis label"));
    arguments += optionFormat2.arg("filename", i18n("Image to save plot to. If empty shows plot in a window on the screen. If inline=true the image is shown regardless of this option."));
    arguments += optionFormat3.arg("filetype", i18n("The image type to save as. One of auto, png, ps, eps, latex, svg, fig."), defaultValue.arg("auto"));
    arguments += optionFormat4.arg("color", i18n("Set to true for colored plot, false for monochrome."), boolean, defaultValue.arg(boolList[QalculateSettings::coloredPlot()]));
    arguments += optionFormat3.arg("xmin", i18n("Minimum x-axis value."), number);
    arguments += optionFormat3.arg("xmax", i18n("Maximum x-axis value."), number);
    arguments += optionFormat4.arg("xlog", i18n("If a logarithimic scale shall be used for the x-axis."), boolean, defaultValue.arg("false"));
    arguments += optionFormat4.arg("ylog", i18n("If a logarithimic scale shall be used for the y-axis."), boolean, defaultValue.arg("false"));
    arguments += optionFormat4.arg("xlogbase", i18n("Logarithimic base for the x-axis."), number, defaultValue.arg("10"));
    arguments += optionFormat4.arg("ylogbase", i18n("Logarithimic base for the y-axis."), boolean, defaultValue.arg("10"));
    arguments += optionFormat4.arg("grid", i18n("If a grid shall be shown in the plot."), boolean, defaultValue.arg(boolList[QalculateSettings::plotGrid()]));
    arguments += optionFormat4.arg("border", i18n("If the plot shall be surrounded by borders on all sides (not just axis)."), boolean, defaultValue.arg(boolList[QalculateSettings::plotBorder()]));
    arguments += optionFormat4.arg("linewidth", i18n("Width of lines."), integer, defaultValue.arg(QString::number(QalculateSettings::plotLineWidth())));
    arguments += optionFormat3.arg("legend", i18n("Where the plot legend shall be placed. One of none, top_left, top_right, bottom_left, bottom_right, below, outside"), defaultValue.arg(legendDefault)); 
    arguments += optionFormat3.arg("smoothing", i18n("Plot smoothing. One of none, unique, csplines, bezier, sbezier"), defaultValue.arg(smoothingDefault)); 
    arguments += optionFormat3.arg("style", i18n("Plot style. One of lines, points, points_lines, boxes, histogram, steps, candlesticks, dots"), defaultValue.arg(styleDefault)); 
    arguments += optionFormat4.arg("xaxis2", i18n("Use scale on second x-axis."), boolean, defaultValue.arg("false"));
    arguments += optionFormat4.arg("yaxis2", i18n("Use scale on second y-axis."), boolean, defaultValue.arg("false"));
    arguments += optionFormat4.arg("inline", i18n("If the plot is to be drawn inline, instead of in a new window."), boolean, defaultValue.arg(boolList[QalculateSettings::inlinePlot()]));
    arguments += optionFormat3.arg("step", i18n("Distance between two interpolation points. See also steps."), number);
    arguments += optionFormat4.arg("steps", i18n("Number of interpolation points. See also step."), integer, defaultValue.arg(QString::number(QalculateSettings::plotSteps())));
    arguments += optionFormat3.arg("xvar", i18n("The name of the x variable. This must be an unknown variable"), defaultValue.arg("x"));
    
    m_answer = title + desc + syntax + arguments;

    
}

void QalculateSyntaxHelpObject::setSaveVariablesInformation()
{
    QString title = "<p>" + i18n("Save variables to a file") + "</p>";
    QString desc = "<p>" + i18n("Save all currently defined variables to a file. They can be reloaded with %1.").arg("loadVariables") + "</p>";
    QString syntax = "<p>saveVariables " + i18n("file") + "</p>";
    QString arguments = "<p>" + i18n("file") + ": " + 
	i18n("the file to save to") + "</p>";
    m_answer = title + desc + syntax + arguments;
}

void QalculateSyntaxHelpObject::setLoadVariablesInformation()
{
    QString title = "<p>" + i18n("Load variables from a file") + "</p>";
    QString desc = "<p>" + i18n("Load variables from a file that has previously been created by %1.").arg("saveVariables") + "</p>";
    QString syntax = "<p>loadVariables " + i18n("file") + "</p>";
    QString arguments = "<p>" + i18n("file") + ": " + 
	i18n("the file to load") + "</p>";
    m_answer = title + desc + syntax + arguments;
}


QString QalculateSyntaxHelpObject::answer()
{
    fetchInformation();
    return m_answer;
}

