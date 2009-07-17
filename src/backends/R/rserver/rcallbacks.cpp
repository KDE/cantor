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

#include "rcallbacks.h"

#include "rserver.h"

#include <kdebug.h>

//R includes
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <Rinterface.h>
#include <R_ext/Parse.h>

#include <stdio.h>

RServer* thread;
Expression* currentExpression;

void setupCallbacks(RServer* r)
{
    kDebug()<<"setting up callbacks";

    thread=r;
    currentExpression=0;

    R_Outputfile=NULL;
    R_Consolefile=NULL;

    ptr_R_WriteConsole=0;
    ptr_R_WriteConsoleEx=onWriteConsoleEx;
    ptr_R_ShowMessage=onShowMessage;
}

void setCurrentExpression(Expression* expr)
{
    currentExpression=expr;
}


enum OutputType { NormalOutput=0, ErrorOutput=1 };

void onWriteConsoleEx(const char* text, int size, int otype)
{
    const QString string=QString::fromUtf8(text, size);
    if (otype==NormalOutput)
    {
        currentExpression->std_buffer+=string;
    }else
    {
        currentExpression->err_buffer+=string;
    }
}

void onShowMessage(const char* text)
{
    const QString string=QString::fromUtf8(text);
    currentExpression->std_buffer+=string;
}
