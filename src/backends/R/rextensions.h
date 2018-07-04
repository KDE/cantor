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
  public Q_SLOTS:
    QString runExternalScript(const QString& path)  override;
    QString scriptFileFilter() override;
    QString highlightingMode() override;
};

class RPlotExtension :  public Cantor::AdvancedPlotExtension,
                        public Cantor::AdvancedPlotExtension::DirectiveAcceptor<Cantor::PlotTitleDirective>,
                        public Cantor::AdvancedPlotExtension::DirectiveAcceptor<Cantor::OrdinateScaleDirective>,
                        public Cantor::AdvancedPlotExtension::DirectiveAcceptor<Cantor::AbscissScaleDirective>
{
  public:
    RPlotExtension(QObject* parent);
    ~RPlotExtension() {}
    QString accept(const Cantor::PlotTitleDirective&) const  override;
    QString accept(const Cantor::OrdinateScaleDirective&) const override;
    QString accept(const Cantor::AbscissScaleDirective&) const override;

  protected:
    QString plotCommand() const  override { return QLatin1String("plot"); }
};

class RVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    RVariableManagementExtension(QObject* parent);
    ~RVariableManagementExtension();
    QString addVariable(const QString& name, const QString& value) Q_DECL_OVERRIDE;
    QString setValue(const QString& name, const QString& value) Q_DECL_OVERRIDE;
    QString removeVariable(const QString& name) Q_DECL_OVERRIDE;
    QString saveVariables(const QString& fileName) Q_DECL_OVERRIDE;
    QString loadVariables(const QString& fileName) Q_DECL_OVERRIDE;
    QString clearVariables() Q_DECL_OVERRIDE;
};

#endif /* _REXTENSIONS_H */
