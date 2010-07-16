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

#include "rextensions.h"

#include <klocale.h>

RScriptExtension::RScriptExtension(QObject* parent) : Cantor::ScriptExtension(parent)
{

}

RScriptExtension::~RScriptExtension()
{

}

QString RScriptExtension::runExternalScript(const QString& path)
{
    return QString("source(\"%1\")").arg(path);
}

QString RScriptExtension::scriptFileFilter()
{
    return i18n("*.R|R script file");
}


// TODO: remove ASAP
QString RPlotExtension::RPlot(const QString& expression,const QString& lab,const QString& xlab, const QString& ylab,bool needXrange,double xmin,double xmax,bool needYrange,double ymin,double ymax)
{
    //Sanitizing the human-readable strings
    QString _lab=QString(lab).replace("\"","\\\\");
    QString _xlab=QString(xlab).replace("\"","\\\\");
    QString _ylab=QString(ylab).replace("\"","\\\\");
    //Composing the command itself
    return QString("plot(%1%2%3%4%5%6)").arg(
        expression,                                                  // Expression itself
        (lab.length()>0)? QString(",main=\"%1\"").arg(lab) : "",     // Main label
        (xlab.length()>0)? QString(",xlab=\"%1\"").arg(xlab) : "",   // X label
        (ylab.length()>0)? QString(",ylab=\"%1\"").arg(ylab) : "",   // Y label
        // Ranges
        (needXrange)? QString(",xlim=range(%1,%2)").arg(QString().setNum(xmin),QString().setNum(xmax)) : "",
        (needYrange)? QString(",ylim=range(%1,%2)").arg(QString().setNum(ymin),QString().setNum(ymax)) : "");
}
