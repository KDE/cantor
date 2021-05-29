/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _REXTENSIONS_H
#define _REXTENSIONS_H

#include "extension.h"
#include "directives/plotdirectives.h"

class RScriptExtension : public Cantor::ScriptExtension
{
  public:
    explicit RScriptExtension(QObject* parent);
    ~RScriptExtension() override = default;
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
    explicit RPlotExtension(QObject* parent);
    ~RPlotExtension() override = default;
    QString accept(const Cantor::PlotTitleDirective&) const  override;
    QString accept(const Cantor::OrdinateScaleDirective&) const override;
    QString accept(const Cantor::AbscissScaleDirective&) const override;

  protected:
    QString plotCommand() const  override { return QLatin1String("plot"); }
};

class RVariableManagementExtension : public Cantor::VariableManagementExtension
{
    public:
    explicit RVariableManagementExtension(QObject* parent);
    ~RVariableManagementExtension() override = default;
    QString addVariable(const QString& name, const QString& value) override;
    QString setValue(const QString& name, const QString& value) override;
    QString removeVariable(const QString& name) override;
    QString saveVariables(const QString& fileName) override;
    QString loadVariables(const QString& fileName) override;
    QString clearVariables() override;
};

#endif /* _REXTENSIONS_H */
