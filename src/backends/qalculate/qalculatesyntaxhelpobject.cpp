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

#include <KLocale>

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
    std::string cmd = command().remove(QLatin1String("help")).simplified().toLatin1().data();
    qDebug() << "HELP CALLED FOR:" << QLatin1String(cmd.c_str());

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
        QString title = i18n("Function: %1", QLatin1String(item->title().c_str()));
        const ExpressionName *ename = &f->preferredName(false);
        int iargs = f->maxargs();
        if(iargs < 0) {
            iargs = f->minargs() + 1;
        }
        QString str,str2,syntax;
        str += QLatin1String(ename->name.c_str());
        str += QLatin1String("(");
        if(iargs != 0) {
            Argument *arg;
            Argument default_arg;
            for(int i2 = 1; i2 <= iargs; i2++) {
                if(i2 > f->minargs()) {
                    str += QLatin1String("[");
                }
                if(i2 > 1) {
                    str += QLatin1String(CALCULATOR->getComma().c_str());
                    str += QLatin1String(" ");
                }
                arg = f->getArgumentDefinition(i2);
                if(arg && !arg->name().empty()) {
                    str2 = QLatin1String(arg->name().c_str());
                } else {
                    str2 = QLatin1String("argument");
                    str2 += QLatin1String(" ");
                    str2 += QString::number(i2);
                }
                str += str2;
                if(i2 > f->minargs()) {
                    str += QLatin1String("]");
                }
            }
            if(f->maxargs() < 0) {
                str += QLatin1String(CALCULATOR->getComma().c_str());
                str += QLatin1String(" ...");
            }
	}
	str += QLatin1String(")");
	syntax = QString::fromLatin1("<p>%1</p>").arg(str);

	QString arguments = QLatin1String("");
        if(iargs != 0) {
            Argument *arg;
            Argument default_arg;
            for(int i2 = 1; i2 <= iargs; i2++) {
                arg = f->getArgumentDefinition(i2);
                if(arg && !arg->name().empty()) {
                    str = QLatin1String(arg->name().c_str());
                } else {
                    str = QString::number(i2);
                }
                str += QLatin1String(": ");
                if(arg) {
                    str2 = QLatin1String(arg->printlong().c_str());
                } else {
                    str2 = QLatin1String(default_arg.printlong().c_str());
                }
                if(i2 > f->minargs()) {
                    str2 += QLatin1String(" (");
                    //optional argument, in description
                    str2 += QLatin1String("optional");
                    if(!f->getDefaultValue(i2).empty()) {
                        str2 += QLatin1String(", ");
                        //argument default, in description
                        str2 += QLatin1String("default: ");
                        str2 += QLatin1String(f->getDefaultValue(i2).c_str());
                    }
                    str2 += QLatin1String(")");
                }
                str += str2;
		arguments += QString::fromLatin1("<p>%1</p>").arg(str);
            }
        }

        QString desc = QString::fromLatin1("<p>%1</p>").arg(QLatin1String(item->description().c_str()));

        m_answer = title + desc + syntax + arguments;
	setHtml(QLatin1String("<p style='white-space:pre'>") + syntax + QLatin1String("</p>"));
	emit done();
    }
}

void QalculateSyntaxHelpObject::setPlotInformation()
{
    QString title = QLatin1String("<p>") + i18n("Plotting interface") + QLatin1String("</p>");
    QString desc = QLatin1String("<p>") + i18n("Plots one or more functions either inline or in a separate window.") + QLatin1String("</p>");
    QString expression = i18n("expression");
    QString option = i18n("option");
    QString value = i18n("value");
    QString syntax = QLatin1String("<p>plot %1 [%2=%3 ...] [, %4 [%5=%6 ...]] ...</p>");
    syntax = syntax.arg(expression, option, value, expression, option, value);

    QString integer = i18n("integer");
    QString boolean = i18n("boolean");
    QString number = i18n("number");
    QString defaultValue = i18n("default: %1");
    QString noDefault = QLatin1String("");
    QString optionFormat2 = QLatin1String("<p>%1: %2</p>");
    QString optionFormat3 = QLatin1String("<p>%1: %2 (%3)</p>");
    QString optionFormat4 = QLatin1String("<p>%1: %2 (%3, %4)</p>");

    QStringList boolList;
    boolList << QLatin1String("false") << QLatin1String("true");

    QString legendDefault;
    QString styleDefault;
    QString smoothingDefault;
    switch (QalculateSettings::plotLegend()) {
    case QalculateSettings::LEGEND_NONE:
	legendDefault = QLatin1String("none"); break;
    case QalculateSettings::LEGEND_TOP_LEFT:
	legendDefault = QLatin1String("top_left"); break;
    case QalculateSettings::LEGEND_TOP_RIGHT:
	legendDefault = QLatin1String("top_right"); break;
    case QalculateSettings::LEGEND_BOTTOM_LEFT:
	legendDefault = QLatin1String("bottom_left"); break;
    case QalculateSettings::LEGEND_BOTTOM_RIGHT:
	legendDefault = QLatin1String("bottom_right"); break;
    case QalculateSettings::LEGEND_BELOW:
	legendDefault = QLatin1String("below"); break;
    case QalculateSettings::LEGEND_OUTSIDE:
	legendDefault = QLatin1String("outside"); break;
    }
    switch(QalculateSettings::plotSmoothing()) {
    case QalculateSettings::SMOOTHING_NONE:
	smoothingDefault = QLatin1String("none");	break;
    case QalculateSettings::SMOOTHING_UNIQUE:
	smoothingDefault = QLatin1String("monotonic"); break;
    case QalculateSettings::SMOOTHING_CSPLINES:
	smoothingDefault = QLatin1String("csplines"); break;
    case QalculateSettings::SMOOTHING_BEZIER:
	smoothingDefault = QLatin1String("bezier"); break;
    case QalculateSettings::SMOOTHING_SBEZIER:
	smoothingDefault = QLatin1String("sbezier"); break;
    }
    switch(QalculateSettings::plotStyle()) {
    case QalculateSettings::STYLE_LINES:
	styleDefault = QLatin1String("lines"); break;
    case QalculateSettings::STYLE_POINTS:
	styleDefault = QLatin1String("points"); break;
    case QalculateSettings::STYLE_LINES_POINTS:
	styleDefault = QLatin1String("points_lines"); break;
    case QalculateSettings::STYLE_BOXES:
	styleDefault = QLatin1String("boxes"); break;
    case QalculateSettings::STYLE_HISTOGRAM:
	styleDefault = QLatin1String("histogram"); break;
    case QalculateSettings::STYLE_STEPS:
	styleDefault = QLatin1String("steps"); break;
    case QalculateSettings::STYLE_CANDLESTICKS:
	styleDefault = QLatin1String("candlesticks"); break;
    case QalculateSettings::STYLE_DOTS:
	styleDefault = QLatin1String("dots"); break;
    }

    QString arguments = QLatin1String("");
    arguments += optionFormat3.arg(QLatin1String("title"), i18n("The function's name"),
				   defaultValue.arg(QLatin1String("expression")));
    arguments += optionFormat2.arg(QLatin1String("plottitle"), i18n("Title label"));
    arguments += optionFormat2.arg(QLatin1String("xlabel"), i18n("x-axis label"));
    arguments += optionFormat2.arg(QLatin1String("ylabel"), i18n("y-axis label"));
    arguments += optionFormat2.arg(QLatin1String("filename"), i18n("Image to save plot to. If empty shows plot in a window on the screen. If inline=true the image is shown regardless of this option."));
    arguments += optionFormat3.arg(QLatin1String("filetype"), i18n("The image type to save as. One of auto, png, ps, eps, latex, svg, fig."), defaultValue.arg(QLatin1String("auto")));
    arguments += optionFormat4.arg(QLatin1String("color"), i18n("Set to true for colored plot, false for monochrome."), boolean, defaultValue.arg(boolList[QalculateSettings::coloredPlot()]));
    arguments += optionFormat3.arg(QLatin1String("xmin"), i18n("Minimum x-axis value."), number);
    arguments += optionFormat3.arg(QLatin1String("xmax"), i18n("Maximum x-axis value."), number);
    arguments += optionFormat4.arg(QLatin1String("xlog"), i18n("If a logarithmic scale shall be used for the x-axis."), boolean, defaultValue.arg(QLatin1String("false")));
    arguments += optionFormat4.arg(QLatin1String("ylog"), i18n("If a logarithmic scale shall be used for the y-axis."), boolean, defaultValue.arg(QLatin1String("false")));
    arguments += optionFormat4.arg(QLatin1String("xlogbase"), i18n("Logarithmic base for the x-axis."), number, defaultValue.arg(QLatin1String("10")));
    arguments += optionFormat4.arg(QLatin1String("ylogbase"), i18n("Logarithmic base for the y-axis."), boolean, defaultValue.arg(QLatin1String("10")));
    arguments += optionFormat4.arg(QLatin1String("grid"), i18n("If a grid shall be shown in the plot."), boolean, defaultValue.arg(boolList[QalculateSettings::plotGrid()]));
    arguments += optionFormat4.arg(QLatin1String("border"), i18n("If the plot shall be surrounded by borders on all sides (not just axis)."), boolean, defaultValue.arg(boolList[QalculateSettings::plotBorder()]));
    arguments += optionFormat4.arg(QLatin1String("linewidth"), i18n("Width of lines."), integer, defaultValue.arg(QString::number(QalculateSettings::plotLineWidth())));
    arguments += optionFormat3.arg(QLatin1String("legend"), i18n("Where the plot legend shall be placed. One of none, top_left, top_right, bottom_left, bottom_right, below, outside"), defaultValue.arg(legendDefault));
    arguments += optionFormat3.arg(QLatin1String("smoothing"), i18n("Plot smoothing. One of none, unique, csplines, bezier, sbezier"), defaultValue.arg(smoothingDefault));
    arguments += optionFormat3.arg(QLatin1String("style"), i18n("Plot style. One of lines, points, points_lines, boxes, histogram, steps, candlesticks, dots"), defaultValue.arg(styleDefault));
    arguments += optionFormat4.arg(QLatin1String("xaxis2"), i18n("Use scale on second x-axis."), boolean, defaultValue.arg(QLatin1String("false")));
    arguments += optionFormat4.arg(QLatin1String("yaxis2"), i18n("Use scale on second y-axis."), boolean, defaultValue.arg(QLatin1String("false")));
    arguments += optionFormat4.arg(QLatin1String("inline"), i18n("If the plot is to be drawn inline, instead of in a new window."), boolean, defaultValue.arg(boolList[QalculateSettings::inlinePlot()]));
    arguments += optionFormat3.arg(QLatin1String("step"), i18n("Distance between two interpolation points. See also steps."), number);
    arguments += optionFormat4.arg(QLatin1String("steps"), i18n("Number of interpolation points. See also step."), integer, defaultValue.arg(QString::number(QalculateSettings::plotSteps())));
    arguments += optionFormat3.arg(QLatin1String("xvar"), i18n("The name of the x variable. This must be an unknown variable"), defaultValue.arg(QLatin1String("x")));

    m_answer = title + desc + syntax + arguments;


}

void QalculateSyntaxHelpObject::setSaveVariablesInformation()
{
    QString title = QLatin1String("<p>") + i18n("Save variables to a file") + QLatin1String("</p>");
    QString desc = QLatin1String("<p>") + i18n("Save all currently defined variables to a file. They can be reloaded with %1.", QLatin1String("loadVariables")) + QLatin1String("</p>");
    QString syntax = QLatin1String("<p>saveVariables ") + i18n("file") + QLatin1String("</p>");
    QString arguments = QLatin1String("<p>") + i18n("file: the file to save to") + QLatin1String("</p>");
    m_answer = title + desc + syntax + arguments;
}

void QalculateSyntaxHelpObject::setLoadVariablesInformation()
{
    QString title = QLatin1String("<p>") + i18n("Load variables from a file") + QLatin1String("</p>");
    QString desc = QLatin1String("<p>") + i18n("Load variables from a file that has previously been created by %1.", QLatin1String("saveVariables")) + QLatin1String("</p>");
    QString syntax = QLatin1String("<p>loadVariables ") + i18n("file") + QLatin1String("</p>");
    QString arguments = QLatin1String("<p>") + i18n("file: the file to load") + QLatin1String("</p>");
    m_answer = title + desc + syntax + arguments;
}


QString QalculateSyntaxHelpObject::answer()
{
    fetchInformation();
    return m_answer;
}

