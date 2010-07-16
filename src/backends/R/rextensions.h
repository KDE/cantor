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

class RScriptExtension : public Cantor::ScriptExtension
{
  public:
    RScriptExtension(QObject* parent);
    ~RScriptExtension();
  public slots:
    virtual QString runExternalScript(const QString& path);
    virtual QString scriptFileFilter();
};

class RPlotExtension : public Cantor::PlotExtension
{
  public:
    RPlotExtension(QObject* parent) : Cantor::PlotExtension(parent) {}
    ~RPlotExtension() {}
  public slots:
    QString plotFunction2d(const QString& function, const QString& variable, const QString& left, const QString& right) { return ""; }
    QString plotFunction3d(const QString& function, VariableParameter var1, VariableParameter var2) { return ""; }
    QString RPlot(const QString& expression,const QString& lab,const QString& xlab, const QString& ylab,bool needXrange,double xmin,double xmax,bool needYrange,double ymin,double ymax);
};

#endif /* _REXTENSIONS_H */
