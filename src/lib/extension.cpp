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

#define EXTENSION_CONSTRUCTORS(name) name::name(QObject* parent) : Extension(#name,parent) {} \
                                     name::~name() {}


Extension::Extension(const QString& name, QObject* parent) : QObject(parent)
{
    setObjectName(name);
}

Extension::~Extension()
{

}

EXTENSION_CONSTRUCTORS(HistoryExtension)
EXTENSION_CONSTRUCTORS(ScriptExtension)
EXTENSION_CONSTRUCTORS(CASExtension)
EXTENSION_CONSTRUCTORS(CalculusExtension)
EXTENSION_CONSTRUCTORS(PlotExtension)
EXTENSION_CONSTRUCTORS(LinearAlgebraExtension)
EXTENSION_CONSTRUCTORS(VariableManagementExtension)

//implement this here, as it's ";" most of the time
QString ScriptExtension::commandSeparator()
{
    return ";\n";
}

//implement this here, as it's "#" most of the time
QString ScriptExtension::commentStartingSequence()
{
    return "#";
}

//implement this here, as it's "" most of the time
QString ScriptExtension::commentEndingSequence()
{
    return "";
}


//some convenience functions, but normally backends have a special command to create
//these matrices/vectors.

QString LinearAlgebraExtension::nullVector(int size, VectorType type)
{
    QStringList values;
    for (int i=0;i<size;i++)
        values<<"0";
    return createVector(values, type);
}

QString LinearAlgebraExtension::identityMatrix(int size)
{
    Matrix m;
    for(int i=0;i<size;i++)
    {
        QStringList column;
        for(int j=0;j<size;j++)
            column<<((i==j) ? "1": "0");

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
            column<<"0";

        m<<column;
    }

    return createMatrix(m);
}
