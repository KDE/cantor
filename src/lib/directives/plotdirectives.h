/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
*/

#ifndef _PLOT_DIRECTIVES_H
#define _PLOT_DIRECTIVES_H

#include "extension.h"
#include "cantor_export.h"

//TODO: comments
namespace Cantor
{
    class CANTOR_EXPORT PlotTitleDirective : public AdvancedPlotExtension::PlotDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(PlotTitleDirective);
            const QString& title() const;
            explicit PlotTitleDirective(const QString& str);
            static AdvancedPlotExtension::DirectiveProducer* widget(QWidget* parent);

        private:
            QString m_title;

    };

    class CANTOR_EXPORT AbstractScaleDirective : public AdvancedPlotExtension::PlotDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(AbstractScaleDirective);
            double min() const;
            double max() const;

        protected:
            AbstractScaleDirective(double a,double b);

        private:
            double m_min;
            double m_max;
    };

    class CANTOR_EXPORT OrdinateScaleDirective : public AbstractScaleDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(OrdinateScaleDirective);
            OrdinateScaleDirective(double a,double b);
            static AdvancedPlotExtension::DirectiveProducer* widget(QWidget* parent);
    };

    class CANTOR_EXPORT AbscissScaleDirective : public AbstractScaleDirective
    {
        public:
            PLOT_DIRECTIVE_DISPATCHING(AbscissScaleDirective);
            AbscissScaleDirective(double a,double b);
            static AdvancedPlotExtension::DirectiveProducer* widget(QWidget* parent);
    };
}

#endif // _PLOT_DIRECTIVES_H
