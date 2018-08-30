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

#include "extension.h"
using namespace Cantor;

#include <QStringList>

#define EXTENSION_CONSTRUCTORS(name) name::name(QObject* parent) : Extension(QLatin1String(#name),parent) {} \
                                     name::~name() {}


Extension::Extension(const QString& name, QObject* parent) : QObject(parent)
{
    setObjectName(name);
}

EXTENSION_CONSTRUCTORS(HistoryExtension)
EXTENSION_CONSTRUCTORS(ScriptExtension)
EXTENSION_CONSTRUCTORS(CASExtension)
EXTENSION_CONSTRUCTORS(CalculusExtension)
EXTENSION_CONSTRUCTORS(PlotExtension)
EXTENSION_CONSTRUCTORS(AdvancedPlotExtension)
EXTENSION_CONSTRUCTORS(LinearAlgebraExtension)
EXTENSION_CONSTRUCTORS(VariableManagementExtension)
EXTENSION_CONSTRUCTORS(PackagingExtension)

//implement this here, as it's ";" most of the time
QString ScriptExtension::commandSeparator()
{
    return QLatin1String(";\n");
}

//implement this here, as it's "#" most of the time
QString ScriptExtension::commentStartingSequence()
{
    return QLatin1String("#");
}

//implement this here, as it's "" most of the time
QString ScriptExtension::commentEndingSequence()
{
    return QLatin1String("");
}


//some convenience functions, but normally backends have a special command to create
//these matrices/vectors.

QString LinearAlgebraExtension::nullVector(int size, VectorType type)
{
    QStringList values;
    for (int i=0;i<size;i++)
        values<<QLatin1String("0");
    return createVector(values, type);
}

QString LinearAlgebraExtension::identityMatrix(int size)
{
    Matrix m;
    for(int i=0;i<size;i++)
    {
        QStringList column;
        for(int j=0;j<size;j++)
            column<<((i==j) ? QLatin1String("1"): QLatin1String("0"));

        m<<column;
    }

    return createMatrix(m);
}

QString LinearAlgebraExtension::nullMatrix(int rows, int columns)
{
    Matrix m;
    for(int i=0;i<rows;i++)
    {
        QStringList column;
        for(int j=0;j<columns;j++)
            column<<QLatin1String("0");

        m<<column;
    }

    return createMatrix(m);
}

QString AdvancedPlotExtension::plotFunction2d(const QString& expression, const QVector<PlotDirective*>& directives) const
{
    QString params = QLatin1String("");
    foreach (PlotDirective* dir, directives)
    {
        QString param=dispatchDirective(*dir);
        if (param.length()>0)
            params+=separatorSymbol()+param;
    }
    return plotCommand() + QLatin1String("(") + expression + params + QLatin1String(")");
}

QString AdvancedPlotExtension::dispatchDirective(const PlotDirective& directive) const
{
    const AcceptorBase* acceptor=dynamic_cast<const AcceptorBase*>(this);
    if (acceptor==nullptr)
    {
        qDebug()<<"Plotting extension does not support any directives, but was asked to process one";
        return QLatin1String("");
    }
    return directive.dispatch(*acceptor);
}

QString AdvancedPlotExtension::separatorSymbol() const
{
    return QLatin1String(",");
}

QWidget* AdvancedPlotExtension::PlotDirective::widget(QWidget* parent)
{
    return new QWidget(parent);
}

AdvancedPlotExtension::AcceptorBase::AcceptorBase() : m_widgets()
{
}

const QVector<AdvancedPlotExtension::AcceptorBase::widgetProc>& AdvancedPlotExtension::AcceptorBase::widgets() const
{
    return m_widgets;
}

AdvancedPlotExtension::DirectiveProducer::DirectiveProducer(QWidget* parent) : QWidget(parent)
{
}
