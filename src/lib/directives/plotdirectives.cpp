/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
*/

#include "directives/plotdirectives.h"
#include "ui_axisrange.h"
#include "ui_plottitle.h"

#include <KLocalizedString>

namespace Cantor
{

// FIXME maybe this belongs to headers rather
class PlotTitleControl : public AdvancedPlotExtension::DirectiveControl<Ui_PlotTitleControl>
{
    public: // FIXME: move window title setting upward, or maybe not
        PlotTitleControl(QWidget *parent) : AbstractParent(parent) { setWindowTitle(ki18n("Main title").toString()); }

        AdvancedPlotExtension::PlotDirective* produceDirective() const override
        {
            return new PlotTitleDirective(titleEdit->text());
        }
};

class AbscissScaleControl : public AdvancedPlotExtension::DirectiveControl<Ui_AxisRangeControl>
{
    public:
        AbscissScaleControl(QWidget *parent) : AbstractParent(parent) { setWindowTitle(ki18n("Abscissa scale").toString()); }

        AdvancedPlotExtension::PlotDirective* produceDirective() const override
        {
            return new AbscissScaleDirective(minEdit->value(),maxEdit->value());
        }
};

class OrdinateScaleControl : public AdvancedPlotExtension::DirectiveControl<Ui_AxisRangeControl>
{
    public:
        OrdinateScaleControl(QWidget *parent) : AbstractParent(parent) { setWindowTitle(ki18n("Ordinate scale").toString()); }

        AdvancedPlotExtension::PlotDirective* produceDirective() const override
        {
            return new OrdinateScaleDirective(minEdit->value(),maxEdit->value());
        }
};


const QString& PlotTitleDirective::title() const
{
    return m_title;
}

PlotTitleDirective::PlotTitleDirective(const QString& str) : m_title(str)
{
}

AdvancedPlotExtension::DirectiveProducer* PlotTitleDirective::widget(QWidget* parent)
{
    return new PlotTitleControl(parent);
}

double AbstractScaleDirective::min() const
{
    return m_min;
}
double AbstractScaleDirective::max() const
{
    return m_max;
}

AbstractScaleDirective::AbstractScaleDirective(double a,double b) : m_min(a),m_max(b)
{
}

AbscissScaleDirective::AbscissScaleDirective(double a,double b) : AbstractScaleDirective(a,b)
{
}

AdvancedPlotExtension::DirectiveProducer* AbscissScaleDirective::widget(QWidget* parent)
{
    return new AbscissScaleControl(parent);
}

OrdinateScaleDirective::OrdinateScaleDirective(double a,double b) : AbstractScaleDirective(a,b)
{
}

AdvancedPlotExtension::DirectiveProducer* OrdinateScaleDirective::widget(QWidget* parent)
{
    return new OrdinateScaleControl(parent);
}

}
