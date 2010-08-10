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

#ifndef _PLOT_DIRECTIVES_H
#define _PLOT_DIRECTIVES_H

#include "extension.h"
#include "cantor_export.h"

//TODO: comments
namespace Cantor
{
    class CANTOR_EXPORT PlotTitleDirective : public Cantor::AdvancedPlotExtension::PlotDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(PlotDirective);
            const QString& title() const { return _title; };
            PlotTitleDirective(const QString& str) : _title(str) {}

        private:
            QString _title;
    };

    class CANTOR_EXPORT AbstractScaleDirective : public Cantor::AdvancedPlotExtension::PlotDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(PlotDirective);
            double min() const { return _min; }
            double max() const { return _max; }

        protected:
            AbstractScaleDirective(double a,double b) : _min(a),_max(b) { }

        private:
            double _min;
            double _max;
    };

    class CANTOR_EXPORT OrdinateScaleDirective : public Cantor::AbstractScaleDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(PlotDirective);
    };

    class CANTOR_EXPORT AbscissScaleDirective : public Cantor::AbstractScaleDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(PlotDirective);
    };
};

#endif // _PLOT_DIRECTIVES_H
