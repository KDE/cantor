/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "rcallbacks.h"

#include "rserver.h"

#include <QDebug>
#include <QStringList>

#include <R_ext/Rdynload.h>
#include <R_ext/Callbacks.h>
#include <Rinternals.h>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <windows.h>
#undef ERROR     // clashes with R
#define Win32    // needed for R includes
#include <R_ext/RStartup.h>
#include <R_ext/Utils.h>
#include <R_ext/libextern.h>

structRstart RK_R_Params;

extern "C" {
	// why oh why isn't Rinterface.h available on Windows?
	LibExtern void* R_GlobalContext;
	LibExtern uintptr_t R_CStackLimit;
	LibExtern void R_SaveGlobalEnvToFile(char*);
}
#else
#include <Rinterface.h>
#endif

#include <stdio.h>

RServer* server;
Expression* currentExpression;

void setupCallbacks(RServer* r)
{
    qDebug()<<"RServer: "<<"setting up callbacks";

    server=r;
    currentExpression=nullptr;

#ifdef Q_OS_WIN
    // Windows uses RStartup structure
    R_DefParams(&RK_R_Params);
    RK_R_Params.R_Quiet = Rboolean::FALSE;
    RK_R_Params.R_Interactive = Rboolean::TRUE;
    RK_R_Params.ReadConsole = onReadConsole;
    RK_R_Params.WriteConsole = nullptr;
    RK_R_Params.WriteConsoleEx = onWriteConsoleEx;
    RK_R_Params.ShowMessage = onShowMessage;
    RK_R_Params.Busy = onBusy;
    // Note: R_SetParams() will be called after R initialization
#else
    // Unix (Linux/macOS) uses function pointers
    R_Outputfile=nullptr;
    R_Consolefile=nullptr;

    ptr_R_WriteConsole=nullptr;
    ptr_R_WriteConsoleEx=onWriteConsoleEx;
    ptr_R_ShowMessage=onShowMessage;
    ptr_R_Busy=onBusy;
    ptr_R_ReadConsole=onReadConsole;
    ptr_R_ShowFiles=onShowFiles;
#endif
}

#ifdef Q_OS_WIN
void connectCallbacks()
{
    // On Windows, activate callbacks after R initialization
    R_SetParams(&RK_R_Params);
}
#endif

void setCurrentExpression(Expression* expr)
{
    currentExpression=expr;
}


enum OutputType { NormalOutput=0, ErrorOutput=1 };

void onWriteConsoleEx(const char* text, int size, int otype)
{
    if (!currentExpression)
        return;  // Ignore output when no expression is active (e.g., during internal commands)

    const QString string=QString::fromUtf8(text, size);
    if (otype==NormalOutput)
    {
        currentExpression->std_buffer+=string;
        server->queuePartialOutput(string);
    }
    else
    {
        // For stderr: show as progress during execution, but also buffer
        // for potential error display at end
        currentExpression->err_buffer+=string;
        server->queuePartialOutput(string);
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

    QString input=server->requestInput(QString::fromUtf8(prompt));
    if (!input.endsWith(QLatin1Char('\n')))
        input.append(QLatin1Char('\n'));

    qDebug()<<"RServer: "<<"got input:"<<input<<"length:"<<input.size()<<"buflen:"<<buflen;

    if(input.size()>buflen)
        input.truncate(buflen);

    QByteArray utf8 = input.toUtf8();
    strcpy((char*) buf, utf8.constData());

    qDebug()<<"RServer: "<<"returning:"<<utf8.size();
    return utf8.size();
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
