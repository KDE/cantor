/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _RCALLBACKS_H
#define _RCALLBACKS_H

//R includes - must come before Qt to avoid macro conflicts
// R_LEGACY_RCOMPLEX avoids non-standard definition in Complex.h that trips up MSVC
#define R_LEGACY_RCOMPLEX
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <R_ext/Parse.h>

// R defines macros that clash with C++ standard library - undef before Qt
#undef length
#undef isNull
#undef warning

#include <QString>

class RServer;

// Expression structure for current command - shared between rserver and rcallbacks
struct Expression {
    QString cmd;
    bool hasOtherResults;
    QString err_buffer;
    QString std_buffer;
};

//This File implements the necessary callbacks for R
//The information gathered will be pushed back to the RThread

void setupCallbacks(RServer* server);
void setCurrentExpression(Expression* expression);

#ifdef Q_OS_WIN
void connectCallbacks();
#endif

void onWriteConsoleEx(const char* text, int size,int otype);
void onShowMessage(const char* text);
void onBusy(int which);
int  onReadConsole(const char* prompt, unsigned char* buf, int buflen, int hist);
int  onShowFiles(int nfile, const char** file, const char** headers, const char* wtitle, Rboolean del, const char* pager);

#endif /* _RCALLBACKS_H */
