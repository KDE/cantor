/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
    return QStringLiteral(";\n");
}

//implement this here, as it's "#" most of the time
QString ScriptExtension::commentStartingSequence()
{
    return QStringLiteral("#");
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
        values<<QStringLiteral("0");
    return createVector(values, type);
}

QString LinearAlgebraExtension::identityMatrix(int size)
{
    Matrix m;
    for(int i=0;i<size;i++)
    {
        QStringList column;
        for(int j=0;j<size;j++)
            column<<((i==j) ? QStringLiteral("1"): QStringLiteral("0"));

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
            column<<QStringLiteral("0");

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
    return QStringLiteral(",");
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
