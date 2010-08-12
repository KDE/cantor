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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _REXTENSIONS_H
#define _REXTENSIONS_H

#include "extension.h"
#include "directives/plotdirectives.h"

class RScriptExtension : public Cantor::ScriptExtension
{
  public:
    RScriptExtension(QObject* parent);
    ~RScriptExtension();
  public slots:
    virtual QString runExternalScript(const QString& path);
    virtual QString scriptFileFilter();
};

class RPlotExtension :  public Cantor::AdvancedPlotExtension,
                        public Cantor::AdvancedPlotExtension::DirectiveAcceptor<Cantor::PlotTitleDirective>,
                        public Cantor::AdvancedPlotExtension::DirectiveAcceptor<Cantor::OrdinateScaleDirective>,
                        public Cantor::AdvancedPlotExtension::DirectiveAcceptor<Cantor::AbscissScaleDirective>
{
  public:
    RPlotExtension(QObject* parent);
    ~RPlotExtension() {}
    QString accept(const Cantor::PlotTitleDirective& directive) const;
    QString accept(const Cantor::OrdinateScaleDirective& directive) const;
    QString accept(const Cantor::AbscissScaleDirective& directive) const;
  protected:
    QString plotCommand() const { return "plot"; }
};

#endif /* _REXTENSIONS_H */
