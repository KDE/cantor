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

#include <QDebug>
#include <QStringList>

#include <stdio.h>
#include <Rinterface.h>

RServer* server;
Expression* currentExpression;

void setupCallbacks(RServer* r)
{
    qDebug()<<"RServer: "<<"setting up callbacks";

    server=r;
    currentExpression=nullptr;

    R_Outputfile=nullptr;
    R_Consolefile=nullptr;

    ptr_R_WriteConsole=nullptr;
    ptr_R_WriteConsoleEx=onWriteConsoleEx;
    ptr_R_ShowMessage=onShowMessage;
    ptr_R_Busy=onBusy;
    ptr_R_ReadConsole=onReadConsole;
    ptr_R_ShowFiles=onShowFiles;
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

void onBusy(int which)
{
    qDebug()<<"RServer: "<<"onBusy: "<<which;
}

int onReadConsole(const char* prompt, unsigned char* buf, int buflen, int hist)
{
    Q_UNUSED(hist);
    qDebug()<<"RServer: "<<"readConsole: "<<prompt;

    QString input=server->requestInput(QLatin1String(prompt));

    if(input.size()>buflen)
        input.truncate(buflen);

    strcpy( (char*) buf, input.toStdString().c_str());

    return input.size();
}

int  onShowFiles(int nfile, const char** file, const char** headers, const char* wtitle, Rboolean del, const char* pager)
{
    int i;
    qDebug()<<"RServer: "<<"show files: ";
    for (i=0;i<nfile; i++)
    {
        qDebug()<<"RServer: "<<"show file "<<file[i]<<" header: "<<headers[i];
    }

    qDebug()<<"RServer: "<<" title: "<<wtitle[i];
    qDebug()<<"RServer: "<<"del: "<<del;
    qDebug()<<"RServer: "<<"pager: "<<pager;

    QStringList files;
    for(int i=0;i<nfile; i++)
        server->addFileToOutput(QString::fromLocal8Bit(file[i]));

    currentExpression->hasOtherResults=true;

    return 0;
}
