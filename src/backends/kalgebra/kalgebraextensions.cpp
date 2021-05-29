/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "kalgebraextensions.h"

KAlgebraVariableManagementExtension::KAlgebraVariableManagementExtension(QObject* parent) : Cantor::VariableManagementExtension(parent)
{

}

QString KAlgebraVariableManagementExtension::addVariable(const QString& name, const QString& value)
{
    //Kalgebra uses the same command for adding and setting a variable
    return setValue(name, value);
}

QString KAlgebraVariableManagementExtension::setValue(const QString& name,const QString& value)
{
    return QString::fromLatin1("%1:=%2").arg(name, value);
}
