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
 */

#include "directives/plotdirectives.h"
#include "ui_axisrange.h"
#include "ui_plottitle.h"

namespace Cantor
{

// FIXME maybe this belongs to headers rather
class PlotTitleControl : public AdvancedPlotExtension::DirectiveControl<Ui_PlotTitleControl>
{
    public:
        PlotTitleControl(QWidget *parent) : AbstractParent(parent) {}

        AdvancedPlotExtension::PlotDirective* produceDirective() const
        {
            return new PlotTitleDirective(titleEdit->text());
        }
};

class AbscissScaleControl : public AdvancedPlotExtension::DirectiveControl<Ui_AxisRangeControl>
{
    public:
        AbscissScaleControl(QWidget *parent) : AbstractParent(parent) {}

        AdvancedPlotExtension::PlotDirective* produceDirective() const
        {
            return new AbscissScaleDirective(minEdit->value(),maxEdit->value());
        }
};

class OrdinateScaleControl : public AdvancedPlotExtension::DirectiveControl<Ui_AxisRangeControl>
{
    public:
        OrdinateScaleControl(QWidget *parent) : AbstractParent(parent) {}

        AdvancedPlotExtension::PlotDirective* produceDirective() const
        {
            return new OrdinateScaleDirective(minEdit->value(),maxEdit->value());
        }
};

AdvancedPlotExtension::DirectiveProducer* PlotTitleDirective::widget(QWidget* parent)
{
    return new PlotTitleControl(parent);
}

AdvancedPlotExtension::DirectiveProducer* AbscissScaleDirective::widget(QWidget* parent)
{
    return new AbscissScaleControl(parent);
}

AdvancedPlotExtension::DirectiveProducer* OrdinateScaleDirective::widget(QWidget* parent)
{
    return new OrdinateScaleControl(parent);
}

}
