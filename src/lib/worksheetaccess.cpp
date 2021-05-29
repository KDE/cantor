/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "worksheetaccess.h"


using namespace Cantor;

QLatin1String WorksheetAccessInterface::Name =  QLatin1String("WorksheetAccessInterface");

WorksheetAccessInterface::WorksheetAccessInterface(QObject* parent) : QObject(parent)
{
    setObjectName(Name);
}



